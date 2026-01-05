#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $0 <samurai_dir> <demo_target>" >&2
  echo "Example: $0 ~/projects/samurai finite-volume-advection-2d" >&2
  exit 1
}

log() {
  printf '[run-samurai] %s\n' "$*"
}

[[ $# -eq 2 ]] || usage

samurai_dir=$(realpath "$1")
demo_target=$2

[[ -d "$samurai_dir" ]] || { echo "Samurai directory '$samurai_dir' not found" >&2; exit 1; }

command -v mpirun >/dev/null || { echo "mpirun not found in PATH" >&2; exit 1; }
command -v cmake >/dev/null || { echo "cmake not found in PATH" >&2; exit 1; }
command -v mpicxx >/dev/null || { echo "mpicxx not found in PATH" >&2; exit 1; }

root_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)
mpip_src_dir="$root_dir/third_party/mpiP"
mpip_install_dir="$mpip_src_dir/install"

[[ -d "$mpip_src_dir" ]] || { echo "mpiP sources not found at '$mpip_src_dir'" >&2; exit 1; }

log "Building mpiP"
pushd "$mpip_src_dir" >/dev/null
export CFLAGS='-DPTR=void* -Dsprintf_vma\(buf\,val\)=bfd_sprintf_vma\(abfd\,buf\,val\)'
./configure --prefix="$mpip_install_dir" CC=mpicc CXX=mpicxx --with-f77=no --with-binutils-dir=/usr
make -j
make install
popd >/dev/null

libmpip="$mpip_install_dir/lib/libmpiP.so"
[[ -f "$libmpip" ]] || { echo "libmpiP.so not found in '$mpip_install_dir/lib'" >&2; exit 1; }

build_dir="$samurai_dir/build"
cxxflags="-g -fno-omit-frame-pointer -fno-pie"
ldflags="-no-pie -rdynamic"

log "Configuring samurai in '$build_dir'"
cmake -S "$samurai_dir" -B "$build_dir" \
  -DWITH_MPI=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_POSITION_INDEPENDENT_CODE=OFF \
  -DCMAKE_CXX_FLAGS="$cxxflags" \
  -DCMAKE_EXE_LINKER_FLAGS="$ldflags"

log "Building demo target '$demo_target'"
cmake --build "$build_dir" --target "$demo_target" -j

log "Searching executable for '$demo_target'"
mapfile -t candidates < <(find "$build_dir" -type f -name "$demo_target" -perm -u+x)

[[ ${#candidates[@]} -gt 0 ]] || { echo "Executable '$demo_target' not found under '$build_dir'" >&2; exit 1; }
if [[ ${#candidates[@]} -gt 1 ]]; then
  echo "Multiple executables named '$demo_target' found:" >&2
  printf '  %s\n' "${candidates[@]}" >&2
  exit 1
fi

demo_exe=${candidates[0]}

log "Running '$demo_exe' with mpiP"
export LD_PRELOAD="$libmpip"
mpip_opts=(-k 8 -n -x "$demo_exe")
if [[ -n ${MPIP_EXTRA_ARGS:-} ]]; then
  # shellcheck disable=SC2206
  extra=($MPIP_EXTRA_ARGS)
  mpip_opts+=("${extra[@]}")
fi
export MPIP="${mpip_opts[*]}"
mpirun -n "${MPI_PROCS:-2}" "$demo_exe"
