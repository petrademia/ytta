package com.ytta.v0;

import com.ytta.v0.Types.AckStatus;
import com.ytta.v0.Types.EngineEvent;
import com.ytta.v0.Types.Liquidity;
import com.ytta.v0.Types.Order;
import com.ytta.v0.Types.Side;

import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertInstanceOf;

class MatchingEngineTest {
    @Test
    void restingNew() {
        MatchingEngine eng = new MatchingEngine();
        OrderGateway gw = new OrderGateway(eng);
        List<EngineEvent> ev = gw.newOrder(new Order(1, Side.BUY, 100, 5), 1000);
        assertEquals(2, ev.size());
        EngineEvent.Ack ack = assertInstanceOf(EngineEvent.Ack.class, ev.get(0));
        assertEquals(AckStatus.ACCEPTED, ack.value().status());
        EngineEvent.Md md = assertInstanceOf(EngineEvent.Md.class, ev.get(1));
        assertEquals(100, md.value().bestBid());
        assertEquals(5, md.value().bestBidQty());
        assertEquals(0, md.value().bestAskQty());
    }

    @Test
    void crossFills() {
        MatchingEngine eng = new MatchingEngine();
        OrderGateway gw = new OrderGateway(eng);
        gw.newOrder(new Order(1, Side.BUY, 100, 5), 1000);
        List<EngineEvent> ev = gw.newOrder(new Order(2, Side.SELL, 100, 5), 2000);
        assertEquals(4, ev.size());
        EngineEvent.Fill maker = assertInstanceOf(EngineEvent.Fill.class, ev.get(1));
        assertEquals(1, maker.value().clOrdId());
        assertEquals(Liquidity.MAKER, maker.value().liquidity());
        assertEquals(100, maker.value().price());
        assertEquals(5, maker.value().qty());
        EngineEvent.Fill taker = assertInstanceOf(EngineEvent.Fill.class, ev.get(2));
        assertEquals(2, taker.value().clOrdId());
        assertEquals(Liquidity.TAKER, taker.value().liquidity());
        EngineEvent.Md md = assertInstanceOf(EngineEvent.Md.class, ev.get(3));
        assertEquals(0, md.value().bestBidQty());
        assertEquals(0, md.value().bestAskQty());
    }

    @Test
    void badCancel() {
        MatchingEngine eng = new MatchingEngine();
        OrderGateway gw = new OrderGateway(eng);
        List<EngineEvent> ev = gw.cancel(99, 1000);
        assertEquals(1, ev.size());
        EngineEvent.Ack ack = assertInstanceOf(EngineEvent.Ack.class, ev.get(0));
        assertEquals(AckStatus.REJECTED, ack.value().status());
        assertEquals("unknown_id", ack.value().reason());
    }

    @Test
    void goodCancel() {
        MatchingEngine eng = new MatchingEngine();
        OrderGateway gw = new OrderGateway(eng);
        gw.newOrder(new Order(3, Side.BUY, 99, 1), 3000);
        List<EngineEvent> ev = gw.cancel(3, 4000);
        assertEquals(2, ev.size());
        EngineEvent.Ack ack = assertInstanceOf(EngineEvent.Ack.class, ev.get(0));
        assertEquals(AckStatus.ACCEPTED, ack.value().status());
        EngineEvent.Md md = assertInstanceOf(EngineEvent.Md.class, ev.get(1));
        assertEquals(0, md.value().bestBidQty());
    }
}
