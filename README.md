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
- HCA, $O(\log p)$ for linear model for the clock drift, $O(p)$ for the clock offset
- HCA2, $O(\log p)$, linear model for the clock drift and clock offset
- HCA3, $O(\log p)$, linear model for the clock drift and clock offset
- HCA3O, $O(\log p)$, fast, only computes clock offset
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


## Support for hwloc

```
-DHWLOC_LIBRARY_DEFAULT_PATH=PATH_TO_HWLOC
```