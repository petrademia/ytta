package com.ytta.v0;

import com.ytta.v0.Types.ActionKind;
import com.ytta.v0.Types.Side;
import com.ytta.v0.Types.StrategyAction;
import com.ytta.v0.Types.Tick;

public final class Strategy {
    private long nextClOrdId = 1;
    private Long cancelTarget = null;

    public StrategyAction onTick(Tick tick) {
        return switch ((int) tick.seq()) {
            case 1 -> {
                long id = nextClOrdId++;
                yield new StrategyAction(
                        ActionKind.NEW_ORDER, tick.tsNs(), id, Side.BUY, 100, 5, "resting_bid");
            }
            case 2 -> {
                long id = nextClOrdId++;
                yield new StrategyAction(
                        ActionKind.NEW_ORDER, tick.tsNs(), id, Side.SELL, 100, 5, "cross_self");
            }
            case 3 -> {
                long id = nextClOrdId++;
                cancelTarget = id;
                yield new StrategyAction(
                        ActionKind.NEW_ORDER, tick.tsNs(), id, Side.BUY, 99, 1, "resting_bid");
            }
            case 4 -> new StrategyAction(
                    ActionKind.CANCEL,
                    tick.tsNs(),
                    cancelTarget == null ? 0 : cancelTarget,
                    Side.BUY,
                    0,
                    0,
                    "done");
            default -> new StrategyAction(
                    ActionKind.NOOP, tick.tsNs(), 0, Side.BUY, 0, 0, "flat");
        };
    }
}
