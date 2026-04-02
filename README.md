# mpi-clock-sync

This repository contains the `mpits` library, which can be used to synchronize 
the clocks of distributed MPI processes.

This is a software solution based on MPI.

## Building `mpits`

```bash
mkdir build
cd build
cmake ..
make -j
```

You may also choose a target directory for the library, e.g.,

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/local/mpits ..
make -j
make install
```

## Selecting a Clock Synchronization Algorithm

The following clock synchronization algorithms are available:
- HCA, builds models bottom-up (leaves to root) in $O(\log p)$ rounds, then remeasures offsets top-down in $O(p)$ rounds
- HCA2, builds models bottom-up in $O(\log p)$ rounds, optionally remeasures offset per round (no separate $O(p)$ pass)
- HCA3, walks top-down (root to leaves) in $O(\log p)$ rounds; each rank composes its model incrementally from its parent
- HCA3O, same top-down tree as HCA3 but measures only a constant clock offset (no linear model for drift); faster but less accurate when clocks drift
- JK, $O(p)$ to compute drift and offset
- SKAMPI, $O(p)$ for clock offset computation (no clock drift)
- TOPO1, meta algorithm, determines hierarchy via hwloc
    - 3 levels, inter-node, node-level, socket-level
- TOPO2, meta two-level algorithm, MPI-only
    - 2 levels, inter-node, node-level
    - one can apply different algorithms to each level
- NONE, no clock synchronization
    - used for passing time through (for time synchronized systems, e.g., PTP, BlueGen/Q, etc.)


```
export MPITS_PARAMS="--clock-sync=HCA3O --params=alg:HCA3O@skampi_offset@5@20"
export MPITS_PARAMS="--clock-sync=HCA3O --params=options:skampi_offset@5@20"
```

### HCA parameter format

```
options:<fitpoints>@<offsetalg>@<p1>@<p2>
```

- `fitpoints` — number of fit points for the linear model
- `offsetalg` — `pingpong_offset` or `skampi_offset`
- `p1`, `p2` — parameters passed to the offset algorithm (min ping-pongs, ping-pongs)

If the format string is missing or incomplete, defaults are used: `fitpoints=500`, `skampi_offset@5@20`.

Example:
```
export MPITS_PARAMS="--clock-sync=HCA --params=options:500@skampi_offset@5@20"
```

### HCA2 parameter format

```
options:<recompute>@<fitpoints>@<offsetalg>@<p1>@<p2>
```

- `recompute` — `1` to recompute the intercept of the linear model, `0` to skip
- `fitpoints` — number of fit points for the linear model
- `offsetalg` — `pingpong_offset` or `skampi_offset` to obtain the fit points for the linear model
- `p1`, `p2` — parameters passed to the offset algorithm (min ping-pongs, ping-pongs)

If the format string is missing or incomplete, defaults are used: `recompute=1`, `fitpoints=500`, `skampi_offset@5@20`.

Example:
```
export MPITS_PARAMS="--clock-sync=HCA2 --params=options:1@500@skampi_offset@5@20"
```

### HCA3 parameter format

```
options:<recompute>@<fitpoints>@<offsetalg>@<p1>@<p2>
```

- `recompute` — `1` to recompute the intercept of the linear model, `0` to skip
- `fitpoints` — number of fit points for the linear model
- `offsetalg` — `pingpong_offset` or `skampi_offset` to obtain the fit points for the linear model
- `p1`, `p2` — parameters passed to the offset algorithm (min ping-pongs, ping-pongs)

If the format string is missing or incomplete, defaults are used: `recompute=1`, `fitpoints=500`, `skampi_offset@5@20`.

Example:
```
export MPITS_PARAMS="--clock-sync=HCA3 --params=options:1@500@skampi_offset@5@20"
```

### JK parameter format

```
options:<fitpoints>@<offsetalg>@<p1>@<p2>
```

- `fitpoints` — number of fit points for the linear model
- `offsetalg` — `pingpong_offset` or `skampi_offset`
- `p1`, `p2` — parameters passed to the offset algorithm

If the format string is missing or incomplete, defaults are used: `fitpoints=1000`, `pingpong_offset@100@100`.

Example:
```
export MPITS_PARAMS="--clock-sync=JK --params=options:1000@pingpong_offset@100@100"
```

### SKaMPI parameter format

```
options:<offsetalg>@<p1>@<p2>
```

- `offsetalg` — `pingpong_offset` or `skampi_offset`
- `p1`, `p2` — parameters passed to the offset algorithm

If the format string is missing or incomplete, defaults are used: `skampi_offset@10@100`.

Example:
```
export MPITS_PARAMS="--clock-sync=SKaMPI --params=options:skampi_offset@10@100"
```

### Topo2 parameter format

`Topo2` does not use a single `options:` string. It expects two named clock-sync configurations:

```text
topoalg1:<alg1-spec>,topoalg2:<alg2-spec>
```

- `topoalg1` — algorithm used for the upper level of the two-level hierarchy
- `topoalg2` — algorithm used for the lower level
- Each value is parsed by the generic clock-sync loader, so it uses the same `ALG@...` format as `alg:...`

Recommended example:

```bash
export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:HCA3@0@500@skampi_offset@10@100,topoalg2:prop@1"
```

This matches the built-in default behavior:
- `topoalg1:HCA3@0@500@skampi_offset@10@100`
- `topoalg2:prop@1`

Examples for `topoalg1`:

```bash
export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:HCA@500@skampi_offset@5@20,topoalg2:prop@1"
export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:HCA2@1@500@skampi_offset@5@20,topoalg2:prop@1"
export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:HCA3@1@500@skampi_offset@5@20,topoalg2:prop@1"
export MPITS_PARAMS="--clock-sync=Topo2 --params=topoalg1:HCA3O@skampi_offset@5@20,topoalg2:prop@1"
```

Notes:
- `prop@0` means offset-only propagation.
- `prop@1` means linear-model propagation.
- With `topoalg1:HCA3O@...`, both `topoalg2:prop@0` and `topoalg2:prop@1` are accepted. `prop@0` is the more consistent pairing because `HCA3O` itself is offset-only.

### HCA3O parameter format

```
options:<offsetalg>@<p1>@<p2>
```

- `offsetalg` — `pingpong_offset` or `skampi_offset`
- `p1`, `p2` — parameters passed to the offset algorithm (min ping-pongs, ping-pongs)

If the format string is missing or incomplete, defaults are used: `skampi_offset@5@20`.

Example:
```
export MPITS_PARAMS="--clock-sync=HCA3O --params=options:skampi_offset@5@20"
```


## Support for hwloc

```
-DHWLOC_LIBRARY_DEFAULT_PATH=PATH_TO_HWLOC
```
