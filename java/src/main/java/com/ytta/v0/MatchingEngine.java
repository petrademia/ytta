package com.ytta.v0;

import com.ytta.v0.Book.MatchSlice;
import com.ytta.v0.Types.AckEvent;
import com.ytta.v0.Types.AckStatus;
import com.ytta.v0.Types.EngineEvent;
import com.ytta.v0.Types.FillEvent;
import com.ytta.v0.Types.Liquidity;
import com.ytta.v0.Types.Op;
import com.ytta.v0.Types.Order;

import java.util.ArrayList;
import java.util.List;

public final class MatchingEngine {
    private final Book book = new Book();

    public List<EngineEvent> submit(Op op, Order order, long tsNs) {
        if (op == Op.CANCEL) {
            if (book.cancel(order.clOrdId()).isEmpty()) {
                return List.of(new EngineEvent.Ack(
                        new AckEvent(tsNs, order.clOrdId(), AckStatus.REJECTED, "unknown_id")));
            }
            List<EngineEvent> events = new ArrayList<>(2);
            events.add(new EngineEvent.Ack(
                    new AckEvent(tsNs, order.clOrdId(), AckStatus.ACCEPTED, "")));
            events.add(new EngineEvent.Md(book.snapshot(tsNs)));
            return events;
        }

        List<MatchSlice> slices = book.match(order.side(), order.price(), order.qty());
        long filled = 0;
        for (MatchSlice s : slices) {
            filled += s.qty();
        }
        long residual = order.qty() - filled;

        List<EngineEvent> events = new ArrayList<>();
        events.add(new EngineEvent.Ack(
                new AckEvent(tsNs, order.clOrdId(), AckStatus.ACCEPTED, "")));
        for (MatchSlice s : slices) {
            events.add(new EngineEvent.Fill(new FillEvent(
                    tsNs, s.makerId(), s.price(), s.qty(), Liquidity.MAKER)));
            events.add(new EngineEvent.Fill(new FillEvent(
                    tsNs, order.clOrdId(), s.price(), s.qty(), Liquidity.TAKER)));
        }
        if (residual > 0) {
            book.add(new Order(order.clOrdId(), order.side(), order.price(), residual));
        }
        events.add(new EngineEvent.Md(book.snapshot(tsNs)));
        return events;
    }
}
