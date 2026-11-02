#!/bin/sh
# input/run_benchmark.sh
# Sweep threads x events x photons x location for geant4 vs celeritas, with
# all optical physics processes enabled (see PhysicsList.cc). Appends rows to
# results.csv and prints each run's log to the terminal.
set -e

app=${1:?usage: run_benchmark.sh <legend_celer executable> <geometry.gdml>}
gdml=${2:?usage: run_benchmark.sh <legend_celer executable> <geometry.gdml>}
# Resolve to absolute paths since each run cd's into its own run_dir
app=$(cd "$(dirname "$app")" && pwd)/$(basename "$app")
gdml=$(cd "$(dirname "$gdml")" && pwd)/$(basename "$gdml")
here=$(cd "$(dirname "$0")" && pwd)
outdir="$here/../output/run"
csv="$here/../output/results.csv"
mkdir -p "$outdir"
[ -f "$csv" ] || echo "mode,threads,events,photons,location,time_sec" > "$csv"

threads_list="1 2"
events_list="1 2"
photons_list="10 100"
# name:x:y:z (mm) gun positions, standing in for "location"
locations="fiber_mid:30:0:-325"

for threads in $threads_list; do
  for events in $events_list; do
    for photons in $photons_list; do
      for loc in $locations; do
        name=$(echo "$loc" | cut -d: -f1)
        x=$(echo "$loc" | cut -d: -f2); y=$(echo "$loc" | cut -d: -f3); z=$(echo "$loc" | cut -d: -f4)

        mac="$outdir/tmp_${threads}_${events}_${photons}_${name}.mac"
        cat > "$mac" <<EOF
/control/verbose 0
/run/verbose 0
/legend/output/file bench.root
/legend/gun/photons $photons
/legend/gun/energy 9.69 eV
/legend/gun/addPosition $x $y $z mm
/run/beamOn $events
EOF

        for mode in geant4 celeritas; do
          disable=; [ "$mode" = geant4 ] && disable=1
          run_dir="$outdir/${mode}_t${threads}_e${events}_p${photons}_${name}"
          mkdir -p "$run_dir"
          if ( cd "$run_dir" && \
               LEGEND_NUM_THREADS=$threads CELER_DISABLE=$disable \
               "$app" "$gdml" "$mac" > log 2>&1 ); then
            cat "$run_dir/log"
          else
            cat "$run_dir/log"
            echo "FAILED: $run_dir" >&2
            continue
          fi

          t=$(grep -o 'Run time: [0-9.eE+-]*' "$run_dir/log" | tail -1 | awk '{print $3}')
          echo "$mode,$threads,$events,$photons,$name,$t" >> "$csv"
        done
      done
    done
  done
done