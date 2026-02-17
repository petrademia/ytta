# Tick-to-Trade Spec (Draft)

## Objective
Process one market tick and produce one trading action within a bounded latency target.

## Inputs
- Tick timestamp
- Instrument identifier
- Side/price/size
- Sequence number

## Outputs
- Action: `NOOP | NEW_ORDER | CANCEL | REPLACE`
- Action timestamp
- Optional reason code

## Non-Functional Targets
- Low p99 end-to-end latency
- Stable tail latency under burst load
- No dropped sequence handling in normal flow
