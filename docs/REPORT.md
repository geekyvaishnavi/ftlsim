# Design and Simulation of a Flash Translation Layer — Results

All numbers below were produced by the simulator in this repository. Reproduce
them with `./scripts/experiments.sh` (add `PREFILL=1` for the prefilled table).

## Method

| Parameter | Value |
|---|---|
| Geometry | 128 blocks × 64 pages = 8,192 physical pages |
| Over-provisioning | 10% → 7,372 logical pages exported |
| Host writes per run | 200,000 |
| GC reserve | 3 clean blocks |
| Seed | 42 (every run is deterministic) |
| Hotspot skew | 90% of accesses into 10% of the address space |

Two starting conditions are reported, because they answer different questions:

- **Steady state (not prefilled).** The device starts empty. Only pages the
  workload actually touches are ever written, so a hotspot run never places cold
  data on the device.
- **Prefilled** (`--prefill`). Every logical page is written once, then the
  counters are reset. The device starts full and holds cold data the measured
  workload never touches — the condition a real SSD spends its life in, and the
  only one where static wear leveling has anything to migrate.

## Result 1 — Write amplification is set by the workload, not the policy

Steady state, dynamic wear leveling:

| Workload | Greedy WAF | Cost-benefit WAF |
|---|---|---|
| Sequential | 1.00 | 1.00 |
| Hotspot | 2.84 | 2.89 |
| Random | 5.48 | 6.00 |

The spread across workloads (1.00 → 5.48) dwarfs the spread across policies
(≤ 0.5). Sequential writing is the degenerate case: consecutive LPNs land in one
block and are invalidated together, so every GC victim is entirely invalid and
copies nothing. WAF is exactly 1.00 and 200,000 host writes cost 3,000 erases —
the theoretical floor of 200,000 ÷ 64 pages per block. Random writing scatters
invalidation across every block, so victims still hold live data that must be
copied before the erase; each host write costs ~5.5 physical writes.

## Result 2 — Over-provisioning dominates everything

Greedy, dynamic wear leveling, steady state:

| Over-provisioning | Random WAF | Hotspot WAF |
|---|---|---|
| 5% | 12.75 | 3.34 |
| 10% | 5.48 | 2.84 |
| 15% | 3.59 | 2.55 |
| 20% | 2.71 | 2.31 |
| 30% | 1.87 | 1.97 |
| 40% | 1.47 | 1.72 |

Going from 5% to 20% spare capacity cuts random-write amplification by 4.7×,
far more than any policy choice achieves. The mechanism is direct: spare
capacity is what lets invalid pages accumulate before a block must be
reclaimed, so victims are dirtier and copy out less. Note the crossover past
30% — once GC is no longer under pressure, the hotspot workload's cold data
still has to be relocated occasionally, while uniform random invalidation keeps
every block equally dirty and easy to reclaim.

## Result 3 — Wear leveling costs almost nothing and works

Prefilled, hotspot workload, greedy GC — the case where cold data exists:

| Wear leveling | WAF | Erase min–max | Spread | σ |
|---|---|---|---|---|
| none | 6.98 | 164–178 | 14 | 2.86 |
| dynamic | 6.98 | 163–174 | 11 | 2.67 |
| static | 6.98 | 166–174 | 8 | 1.90 |

With a tighter hotspot (5% of the space taking 95% of writes) the separation is
sharper — erase spread 38 → 23 → 13 for none → dynamic → static — at a WAF cost
of 7.83 → 7.81. Dynamic leveling is free: it only changes *which* free block is
opened next, never how many writes occur. Static leveling does add relocations
(it moves cold data so its blocks re-enter the pool), but at the default
interval the cost is under 0.3% of physical writes.

Under a **sequential** workload every policy is identical (spread 24–25, σ 0.47)
because the allocation order already sweeps the array uniformly. Wear leveling
is machinery for skewed workloads; on uniform ones it has nothing to correct.

## Result 4 — Cost-benefit loses to greedy here, and the reason is structural

Cost-benefit is worse on WAF than greedy in every non-sequential configuration
measured (random 6.00 vs 5.48; hotspot 2.89 vs 2.84), while producing a *better*
erase distribution (hotspot prefilled: σ 2.00 vs 2.67).

This is the opposite of the usual published result, and the cause is a missing
mechanism rather than a bad formula. Cost-benefit's advantage comes from
preferring old, cold blocks so that hot data is not repeatedly recollected — but
that only pays off if relocated cold pages are written somewhere *separate* from
incoming hot host writes. This FTL has a single write frontier: GC copy-outs and
host writes share one open block. Cold pages relocated by cost-benefit are
immediately re-mixed with hot data, so they get dragged into the next collection
and the policy pays the copy cost without collecting the benefit.

**The fix is hot/cold separation** — a second open block reserved for GC
relocations — which is the natural next extension of this simulator. Until then,
greedy is the better default, and the honest conclusion is that victim selection
cannot be evaluated independently of data placement.

## Full matrices

Steady state (not prefilled), 200,000 host writes:

| Workload | GC policy | Wear leveling | WAF | Erases | Erase min–max | σ |
|---|---|---|---|---|---|---|
| sequential | greedy | none | 1.00 | 3000 | 23–24 | 0.50 |
| sequential | greedy | dynamic | 1.00 | 3000 | 23–24 | 0.50 |
| sequential | greedy | static | 1.00 | 3000 | 23–24 | 0.50 |
| sequential | cost-benefit | none | 1.00 | 3000 | 23–24 | 0.50 |
| sequential | cost-benefit | dynamic | 1.00 | 3000 | 23–24 | 0.50 |
| sequential | cost-benefit | static | 1.00 | 3000 | 23–24 | 0.50 |
| random | greedy | none | 5.49 | 17037 | 129–137 | 1.50 |
| random | greedy | dynamic | 5.48 | 17005 | 129–134 | 1.07 |
| random | greedy | static | 5.48 | 17005 | 129–134 | 1.07 |
| random | cost-benefit | none | 6.01 | 18649 | 139–153 | 2.84 |
| random | cost-benefit | dynamic | 6.00 | 18615 | 140–148 | 1.89 |
| random | cost-benefit | static | 6.01 | 18671 | 141–148 | 1.73 |
| hotspot | greedy | none | 2.84 | 8765 | 63–74 | 2.32 |
| hotspot | greedy | dynamic | 2.84 | 8750 | 61–71 | 2.10 |
| hotspot | greedy | static | 2.84 | 8747 | 63–72 | 1.67 |
| hotspot | cost-benefit | none | 2.90 | 8925 | 65–77 | 1.98 |
| hotspot | cost-benefit | dynamic | 2.89 | 8918 | 66–72 | 1.27 |
| hotspot | cost-benefit | static | 2.89 | 8918 | 66–72 | 1.27 |

Prefilled, 200,000 host writes after the fill:

| Workload | GC policy | Wear leveling | WAF | Erases | Erase min–max | σ |
|---|---|---|---|---|---|---|
| sequential | greedy | none | 1.00 | 3116 | 24–25 | 0.47 |
| sequential | greedy | dynamic | 1.00 | 3116 | 24–25 | 0.47 |
| sequential | greedy | static | 1.00 | 3116 | 24–25 | 0.47 |
| sequential | cost-benefit | none | 1.00 | 3116 | 24–25 | 0.47 |
| sequential | cost-benefit | dynamic | 1.00 | 3116 | 24–25 | 0.47 |
| sequential | cost-benefit | static | 1.00 | 3116 | 24–25 | 0.47 |
| random | greedy | none | 6.01 | 18759 | 143–151 | 1.40 |
| random | greedy | dynamic | 6.01 | 18760 | 144–148 | 0.97 |
| random | greedy | static | 6.01 | 18760 | 144–148 | 0.97 |
| random | cost-benefit | none | 6.60 | 20608 | 151–168 | 3.32 |
| random | cost-benefit | dynamic | 6.61 | 20640 | 153–164 | 1.99 |
| random | cost-benefit | static | 6.61 | 20657 | 156–164 | 1.71 |
| hotspot | greedy | none | 6.98 | 21808 | 164–178 | 2.86 |
| hotspot | greedy | dynamic | 6.98 | 21814 | 163–174 | 2.67 |
| hotspot | greedy | static | 6.98 | 21805 | 166–174 | 1.90 |
| hotspot | cost-benefit | none | 7.35 | 22944 | 171–191 | 4.34 |
| hotspot | cost-benefit | dynamic | 7.35 | 22969 | 172–182 | 2.00 |
| hotspot | cost-benefit | static | 7.35 | 22954 | 174–182 | 1.99 |

Prefilling raises hotspot WAF from 2.84 to 6.98. The workload is unchanged; what
changed is that the device now holds cold valid data, so every GC victim carries
live pages that must be copied. This is why SSD write-amplification figures
measured on a fresh drive are not comparable to figures from a used one.

## Threats to validity

- Single seed per configuration. Differences under ~2% should not be read as
  real; the policy gaps reported above are larger than that, the wear-leveling
  WAF costs are not.
- No concurrency, channel, or plane parallelism modeling — this counts page
  programs and erases, not time. "Performance" here means write amplification
  and wear, never latency or IOPS.
- Synthetic workloads only. Real traces have spatial locality and idle periods
  that neither the hotspot nor the random generator reproduces, and idle time in
  particular is what real drives use to hide GC.
- The single-write-frontier limitation in Result 4 affects the GC policy
  comparison specifically, not the workload or over-provisioning results.
