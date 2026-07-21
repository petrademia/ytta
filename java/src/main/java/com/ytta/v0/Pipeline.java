package com.ytta.v0;

import com.ytta.v0.Types.AckStatus;
import com.ytta.v0.Types.ActionKind;
import com.ytta.v0.Types.EngineEvent;
import com.ytta.v0.Types.Order;
import com.ytta.v0.Types.StrategyAction;
import com.ytta.v0.Types.Tick;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

public final class Pipeline {
    public enum GoldenType {
        MD(0),
        ACTION(1),
        ACK(2),
        FILL(3);

        final int rank;

        GoldenType(int rank) {
            this.rank = rank;
        }
    }

    public record GoldenEvent(long tsNs, GoldenType type, long clOrdId, String line) {}

    public static String formatAction(StrategyAction a) {
        StringBuilder sb = new StringBuilder();
        sb.append("{\"type\":\"action\",\"ts_ns\":").append(a.tsNs()).append(",\"action\":\"");
        switch (a.kind()) {
            case NOOP -> sb.append("NOOP");
            case NEW_ORDER -> sb.append("NEW_ORDER");
            case CANCEL -> sb.append("CANCEL");
        }
        sb.append('"');
        switch (a.kind()) {
            case NEW_ORDER -> sb.append(",\"cl_ord_id\":")
                    .append(a.clOrdId())
                    .append(",\"side\":\"")
                    .append(a.side().wire())
                    .append("\",\"price\":")
                    .append(a.price())
                    .append(",\"qty\":")
                    .append(a.qty());
            case CANCEL -> sb.append(",\"cl_ord_id\":").append(a.clOrdId());
            case NOOP -> {}
        }
        sb.append(",\"reason\":\"").append(a.reason()).append("\"}");
        return sb.toString();
    }

    public static GoldenEvent formatEngineEvent(EngineEvent e, long tsNs) {
        if (e instanceof EngineEvent.Ack ackEv) {
            var ack = ackEv.value();
            StringBuilder sb = new StringBuilder();
            sb.append("{\"type\":\"ack\",\"ts_ns\":")
                    .append(ack.tsNs())
                    .append(",\"cl_ord_id\":")
                    .append(ack.clOrdId())
                    .append(",\"status\":\"")
                    .append(ack.status().wire())
                    .append('"');
            if (ack.status() == AckStatus.REJECTED) {
                sb.append(",\"reason\":\"").append(ack.reason()).append('"');
            }
            sb.append('}');
            return new GoldenEvent(tsNs, GoldenType.ACK, ack.clOrdId(), sb.toString());
        }
        if (e instanceof EngineEvent.Fill fillEv) {
            var fill = fillEv.value();
            return new GoldenEvent(
                    tsNs,
                    GoldenType.FILL,
                    fill.clOrdId(),
                    "{\"type\":\"fill\",\"ts_ns\":"
                            + fill.tsNs()
                            + ",\"cl_ord_id\":"
                            + fill.clOrdId()
                            + ",\"price\":"
                            + fill.price()
                            + ",\"qty\":"
                            + fill.qty()
                            + ",\"liquidity\":\""
                            + fill.liquidity().wire()
                            + "\"}");
        }
        var md = ((EngineEvent.Md) e).value();
        return new GoldenEvent(
                tsNs,
                GoldenType.MD,
                0,
                "{\"type\":\"md\",\"ts_ns\":"
                        + md.tsNs()
                        + ",\"instrument\":\""
                        + md.instrument()
                        + "\",\"best_bid\":"
                        + md.bestBid()
                        + ",\"best_ask\":"
                        + md.bestAsk()
                        + ",\"best_bid_qty\":"
                        + md.bestBidQty()
                        + ",\"best_ask_qty\":"
                        + md.bestAskQty()
                        + "}");
    }

    public static void sortGolden(List<GoldenEvent> events) {
        events.sort(Comparator
                .comparingLong(GoldenEvent::tsNs)
                .thenComparingInt(e -> e.type().rank)
                .thenComparingLong(GoldenEvent::clOrdId));
    }

    public static List<GoldenEvent> run(List<Tick> ticks, LatencyProbe probe) {
        MatchingEngine engine = new MatchingEngine();
        OrderGateway gateway = new OrderGateway(engine);
        Strategy strategy = new Strategy();
        List<GoldenEvent> out = new ArrayList<>();

        for (Tick tick : ticks) {
            long t0 = System.nanoTime();
            StrategyAction action = strategy.onTick(tick);
            long cl = action.kind() == ActionKind.NOOP ? 0 : action.clOrdId();
            out.add(new GoldenEvent(action.tsNs(), GoldenType.ACTION, cl, formatAction(action)));

            if (action.kind() != ActionKind.NOOP) {
                List<EngineEvent> events = switch (action.kind()) {
                    case NEW_ORDER -> gateway.newOrder(
                            new Order(action.clOrdId(), action.side(), action.price(), action.qty()),
                            action.tsNs());
                    case CANCEL -> gateway.cancel(action.clOrdId(), action.tsNs());
                    case NOOP -> List.of();
                };
                for (EngineEvent ev : events) {
                    out.add(formatEngineEvent(ev, action.tsNs()));
                }
            }

            probe.recordNs(System.nanoTime() - t0);
        }

        sortGolden(out);
        return out;
    }
}
