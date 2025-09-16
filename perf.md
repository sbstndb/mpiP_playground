# Profiling Insights – `finite-volume-advection-2d`

Dernière exécution : `finite-volume-advection-2d.2.316127.1.mpiP` (MPI ~8.0 %). Les trois `MPI_Allreduce` les plus chers consomment ~25 % du temps MPI total.

| Callsite | Temps MPI (%) | Contexte | Optimisations possibles |
| --- | --- | --- | --- |
| 361 (`MPI_Allreduce` – max level) | 14.8 % | `boost::mpi::all_reduce_impl` → `samurai::MRMesh::update_sub_mesh_impl` (`mesh.hpp:164`) → `Mesh_base::update_sub_mesh` (`mesh.hpp:782`) → `Adapt::operator()` (`adapt.hpp:162`) | 1) Mémoïser le couple (min/max) tant que la structure (`mesh == new_mesh`) ne change pas. 2) Remplacer le duo min/max par un seul appel vectorisé (`all_reduce` sur `std::array{min,max}`) pour amortir l’appel MPI. 3) Étudier l’usage de `MPI_Iallreduce` pour recouvrir la réduction par la préparation des tags. |
| 327 (`MPI_Allreduce` – bool converge) | 9.8 % | `boost::mpi::all_reduce_impl<bool, logical_or>` → `samurai::Adapt::harten` (`adapt.hpp:383`) → `Adapt::operator()` (`adapt.hpp:162`) | 1) Ne réduire qu’après plusieurs itérations du tag (les détails peuvent être vérifiés localement avant de globaliser). 2) Utiliser `MPI_Allreduce` groupé (joint avec les min/max) ou `MPI_Allgather` pour récupérer les flags en une fois. |
| 6 (`MPI_Allreduce` – mesh equality check) | 7.9 % | `mpi::all_reduce` dans `Adapt::harten` (`adapt.hpp:383`) via `update_ghost_mr_if_needed` | 1) Détecter localement si un level a changé et ne globaliser que si nécessaire. 2) `MPI_Allreduce` sur un `std::uint64_t` compressant plusieurs booleans (bitmask). |

# Profiling Insights – `finite-volume-burgers` (options `--max-level 8 --min-level 5 --Tf 0.1 --timers --nfiles 1`)

Rapport : `finite-volume-burgers.2.318538.1.mpiP`. MPI ne représente que ~2.3 % du temps total, dominé par l’écriture HDF5.

| Bottleneck | Temps MPI (%) | Contexte | Pistes |
| --- | --- | --- | --- |
| `MPI_File_set_view` (site 295) | 2.10 % | `samurai::Hdf5` ctor (`hdf5.hpp:488`) → `save` (`hdf5.hpp:1047`) | Regrouper les sorties (moins de `set_view`), conserver un layout persistant, limiter le nombre de snapshots. |
| `MPI_File_write_at_all` (site 295) | 2.01 % | Même flux HDF5 | Bufferiser les données, écrire en blocs plus gros, désactiver les appels inutiles via l’option `--timers` uniquement pour debug. |
| `MPI_Allgather` / `MPI_Waitall` (sites 301/296) | ~1 % | `update_ghost_subdomains` (`update.hpp:716`) → `update_ghost_mr_if_needed` → `burgers.cpp:269` | Regrouper les halos (persistant + `MPI_Neighbor_alltoallw`), éviter `all_gather` quand la topologie est à 2 voisins. |

# Recommandations générales

1. **Réduction du nombre de `all_reduce`** : fusionner les appels min/max/flags en un seul vecteur ; limiter les synchronisations globales dans `Adapt::operator()`.
2. **Recouvrement** : passer aux collectives non bloquantes (`MPI_Iallreduce`) autour des phases CPU lourdes.
3. **Profiling HDF5** : en cas de sorties fréquentes, préférer un format local ou différer l’écriture (I/O offload).
