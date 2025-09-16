# MPI Bottleneck Profiling with mpiP

Ce projet minimal montre comment provoquer un goulot d'étranglement (bottleneck) dans une application MPI C++ et l'observer avec l'outil de profilage [mpiP](https://github.com/LLNL/mpiP).

## 1. Pré-requis

- Un environnement MPI (OpenMPI suffit).
- `cmake`, `make`, un compilateur C++ compatible.

## 2. Construction de mpiP

Assurez-vous d'avoir les en-têtes binutils (`binutils-dev`) pour permettre la traduction des symboles :

```bash
cd third_party/mpiP
export CFLAGS='-DPTR=void* -Dsprintf_vma\(buf\,val\)=bfd_sprintf_vma\(abfd\,buf\,val\)'
./configure --prefix="$PWD/install" CC=mpicc CXX=mpicxx --with-f77=no --with-binutils-dir=/usr
make -j
make install
cd ../..
```

L'installation locale place la bibliothèque partagée dans `third_party/mpiP/install/lib` et active la résolution via `bfd`.

> Pourquoi ces `CFLAGS` ? Les versions récentes de binutils (>2.40) ne fournissent plus les alias historiques `PTR` et `sprintf_vma` attendus par mpiP 3.5. Les définir côté compilation (plutôt que patcher les sources) permet de rester sur une copie upstream.

## 3. Compilation des exemples MPI

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
```

Les cibles `bottleneck` et `nested_bottleneck` sont liées automatiquement à `mpiP`, conservent le frame pointer et sont générées en mode non-PIE pour faciliter la remontée de pile.

## 4. Exécution et profilage

```bash
export MPIP="-k 4 -n -x $(pwd)/build/bottleneck"
mpirun -n 4 ./build/bottleneck

# Exemple plus riche
export MPIP="-k 6 -n -v -x $(pwd)/build/nested_bottleneck"
mpirun -n 4 ./build/nested_bottleneck
```

L'outil `mpiP` génère un rapport `bottleneck.<tâches>.<pid>.<id>.mpiP` dans le répertoire courant. Les sections `MPI Time` et `Aggregate Time` mettent en évidence le temps passé dans les appels bloquants (`MPI_Ssend`, `MPI_Barrier`, etc.), illustrant le goulot d'étranglement dû au traitement séquentiel effectué par le rang 0.

La variable d'environnement `MPIP` permet de configurer le profiler :

- `-k n` augmente la profondeur de pile reportée par callsite.
- `-n` évite la troncature des chemins.
- `-x <exe>` indique explicitement où trouver l'exécutable.
- `-v` active simultanément le rapport concis et détaillé.
- `-p`/`-y` ajoutent des histogrammes de messages point-à-point/collectifs.

Le binaire `bottleneck` délègue désormais tout le traitement à `src/simple/` (`pipeline.cpp`, `workload.cpp`). Les rapports mpiP affichent ainsi clairement la pile utilisateur (`simple::root_receive_loop → simple::run_rank → main`).

Le projet `nested_bottleneck` (voir `src/nested/`) répartit la logique dans plusieurs fichiers et imbrique plusieurs couches d'appels avant d'atteindre `MPI_Allgather`, `MPI_Allreduce` ou `MPI_Bcast`. Les rapports mpiP présentent une pile encore plus détaillée reliant les fonctions C++ métier à chaque appel MPI. Ajustez `-k` pour conserver la profondeur souhaitée (ex. `-k 4` pour un aperçu compact, `-k 6` pour voir toute la chaîne `nested::detail::* → nested::run_pipeline → main`).
