package com.ytta.v0;

import com.ytta.v0.Types.EngineEvent;
import com.ytta.v0.Types.Op;
import com.ytta.v0.Types.Order;
import com.ytta.v0.Types.Side;

import java.util.List;

public final class OrderGateway {
    private final MatchingEngine engine;

    public OrderGateway(MatchingEngine engine) {
        this.engine = engine;
    }

    public List<EngineEvent> newOrder(Order order, long tsNs) {
        return engine.submit(Op.NEW, order, tsNs);
    }

    public List<EngineEvent> cancel(long clOrdId, long tsNs) {
        return engine.submit(Op.CANCEL, new Order(clOrdId, Side.BUY, 0, 0), tsNs);
    }
}
