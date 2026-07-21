package com.ytta.v0;

import com.ytta.v0.Types.MdEvent;
import com.ytta.v0.Types.Order;
import com.ytta.v0.Types.Side;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.OptionalLong;
import java.util.TreeMap;

final class Book {
    private record LevelOrder(long clOrdId, long qty) {
        LevelOrder withQty(long q) {
            return new LevelOrder(clOrdId, q);
        }
    }

    record MatchSlice(long makerId, long price, long qty) {}

    // bids: highest price first via reverse order
    private final TreeMap<Long, Deque<LevelOrder>> bids = new TreeMap<>((a, b) -> Long.compare(b, a));
    // asks: lowest price first
    private final TreeMap<Long, Deque<LevelOrder>> asks = new TreeMap<>();
    private final Map<Long, Side> sideById = new HashMap<>();

    void add(Order order) {
        LevelOrder level = new LevelOrder(order.clOrdId(), order.qty());
        TreeMap<Long, Deque<LevelOrder>> levels = order.side() == Side.BUY ? bids : asks;
        levels.computeIfAbsent(order.price(), p -> new ArrayDeque<>()).addLast(level);
        sideById.put(order.clOrdId(), order.side());
    }

    OptionalLong cancel(long clOrdId) {
        Side side = sideById.get(clOrdId);
        if (side == null) {
            return OptionalLong.empty();
        }
        TreeMap<Long, Deque<LevelOrder>> levels = side == Side.BUY ? bids : asks;
        Long foundPx = null;
        Long foundQty = null;
        for (Map.Entry<Long, Deque<LevelOrder>> e : levels.entrySet()) {
            Deque<LevelOrder> q = e.getValue();
            var it = q.iterator();
            while (it.hasNext()) {
                LevelOrder o = it.next();
                if (o.clOrdId() == clOrdId) {
                    it.remove();
                    foundPx = e.getKey();
                    foundQty = o.qty();
                    break;
                }
            }
            if (foundPx != null) {
                break;
            }
        }
        if (foundPx == null) {
            return OptionalLong.empty();
        }
        Deque<LevelOrder> q = levels.get(foundPx);
        if (q == null || q.isEmpty()) {
            levels.remove(foundPx);
        }
        sideById.remove(clOrdId);
        return OptionalLong.of(foundQty);
    }

    List<MatchSlice> match(Side aggressor, long price, long qty) {
        List<MatchSlice> slices = new ArrayList<>();
        long remaining = qty;
        if (aggressor == Side.BUY) {
            while (remaining > 0 && !asks.isEmpty()) {
                Map.Entry<Long, Deque<LevelOrder>> best = asks.firstEntry();
                if (best.getKey() > price) {
                    break;
                }
                remaining = matchLevel(asks, best.getKey(), remaining, slices);
            }
        } else {
            while (remaining > 0 && !bids.isEmpty()) {
                Map.Entry<Long, Deque<LevelOrder>> best = bids.firstEntry();
                if (best.getKey() < price) {
                    break;
                }
                remaining = matchLevel(bids, best.getKey(), remaining, slices);
            }
        }
        return slices;
    }

    private long matchLevel(
            TreeMap<Long, Deque<LevelOrder>> levels,
            long px,
            long remaining,
            List<MatchSlice> slices) {
        Deque<LevelOrder> q = levels.get(px);
        while (remaining > 0 && q != null && !q.isEmpty()) {
            LevelOrder head = q.peekFirst();
            long fillQty = Math.min(remaining, head.qty());
            slices.add(new MatchSlice(head.clOrdId(), px, fillQty));
            long left = head.qty() - fillQty;
            remaining -= fillQty;
            if (left == 0) {
                q.removeFirst();
                sideById.remove(head.clOrdId());
            } else {
                q.removeFirst();
                q.addFirst(head.withQty(left));
            }
        }
        if (q == null || q.isEmpty()) {
            levels.remove(px);
        }
        return remaining;
    }

    MdEvent snapshot(long tsNs) {
        long bestBid = 0;
        long bestBidQty = 0;
        long bestAsk = 0;
        long bestAskQty = 0;
        if (!bids.isEmpty()) {
            Map.Entry<Long, Deque<LevelOrder>> e = bids.firstEntry();
            bestBid = e.getKey();
            for (LevelOrder o : e.getValue()) {
                bestBidQty += o.qty();
            }
        }
        if (!asks.isEmpty()) {
            Map.Entry<Long, Deque<LevelOrder>> e = asks.firstEntry();
            bestAsk = e.getKey();
            for (LevelOrder o : e.getValue()) {
                bestAskQty += o.qty();
            }
        }
        return new MdEvent(tsNs, "INSTR1", bestBid, bestAsk, bestBidQty, bestAskQty);
    }
}
