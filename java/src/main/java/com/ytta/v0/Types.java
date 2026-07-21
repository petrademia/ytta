package com.ytta.v0;

import java.util.ArrayList;
import java.util.List;

public final class Types {
    private Types() {}

    public enum Side {
        BUY,
        SELL;

        public String wire() {
            return name();
        }
    }

    public enum Op {
        NEW,
        CANCEL
    }

    public enum AckStatus {
        ACCEPTED,
        REJECTED;

        public String wire() {
            return name();
        }
    }

    public enum Liquidity {
        MAKER,
        TAKER;

        public String wire() {
            return name();
        }
    }

    public enum ActionKind {
        NOOP,
        NEW_ORDER,
        CANCEL
    }

    public record Order(long clOrdId, Side side, long price, long qty) {}

    public record AckEvent(long tsNs, long clOrdId, AckStatus status, String reason) {}

    public record FillEvent(long tsNs, long clOrdId, long price, long qty, Liquidity liquidity) {}

    public record MdEvent(
            long tsNs,
            String instrument,
            long bestBid,
            long bestAsk,
            long bestBidQty,
            long bestAskQty) {}

    public sealed interface EngineEvent permits EngineEvent.Ack, EngineEvent.Fill, EngineEvent.Md {
        record Ack(AckEvent value) implements EngineEvent {}

        record Fill(FillEvent value) implements EngineEvent {}

        record Md(MdEvent value) implements EngineEvent {}
    }

    public record Tick(long tsNs, String instrument, Side side, long price, long qty, long seq) {}

    public record StrategyAction(
            ActionKind kind,
            long tsNs,
            long clOrdId,
            Side side,
            long price,
            long qty,
            String reason) {}

    public static List<EngineEvent> listOf(EngineEvent... events) {
        List<EngineEvent> out = new ArrayList<>(events.length);
        for (EngineEvent e : events) {
            out.add(e);
        }
        return out;
    }
}
