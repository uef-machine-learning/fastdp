
The RKNNG software implements the following algorithms for k-nearest neighbor graph construction:
 - Random Pair Divisive Construction of kNN graph (RP-Div) [1]
 - Nearest Neighbor Descent (NNDES) [2]

The software is provided with GNU Lesser General Public License, Version 3. https://www.gnu.org/licenses/lgpl.html. All files except those under data and contrib -folders are subject to this license. See LICENSE.txt.

========
Compile:

make rknng

(Tested in Ubuntu Linux 16.04)


====================
Command line options

rknng [--help] [--delta=<STOP>] [--distpar=<FLOAT>] [-k <n>] [--seed=<n>] [--type=<vec|txt>] [--dfunc=<FUNC>] [--format=<ascii|lshkit>] [--outf=<format>] [--algo=<name>] [-o <file>] [--gt=<file>] <file>
  --help                    display this help and exit
  --delta=<STOP>            Stop when delta < STOP 
  --distpar=<FLOAT>         Parameter to distance function (minkowski p-value)
  -k, --num_neighbors=<n>   number of neighbors
  --seed=<n>                random number seed
  --type=<vec|txt>          Input data type: vectorial or text.
  --dfunc=<FUNC>            Distance function:
     l2 = euclidean distance (vectorial, default)
     mnkw = Minkowski distance (vectorial)
     lev = Levenshtein distance (for strings, default)
     dice = Dice coefficient / bigrams (for strings)

  --format=<ascii|lshkit>   Input format: ascii or lshkit (binary)
  --outf=<format>           Output format: {txt,ivec,wgraph}
  --algo=<name>             Algorithm: {rpdiv,nndes}
  -o, --out=<file>          output file
  --gt=<file>               Ground truth graph file (ivec)
  <file>                    input files

==================
Input Data formats

lshkit (input data, numerical):

From http://lshkit.sourceforge.net/dc/d46/matrix_8h.html :
"In the beginning of the file are three 32-bit unsigned integers: ELEM-SIZE, SIZE, DIM. ELEM-SIZE is the size of the element, and currently the only valid value is 4, which is the size of float. SIZE is the number of vectors in the file and DIM is the dimension. The rest of the file is SIZE vectors stored consecutively, and the total size is SIZE * DIM * 4 bytes."

ascii (input data, numerical): Float values separated by spaces, vectors separated by lines.
Example:
1   1.7450209e-01   1.3405977e-01   2.2567669e-01  1.4958786e-01
2.4396723e-02   3.8201822e-01   2.8091144e-01   8.7233654e-02   1.6419012e-01


===================
Output Data formats

txt (graph format):
Header (first line): number of nodes.
Other lines: One line for each item in dataset. Line format:
 - First number: id of node (in range 0..(N-1))
 - Second number: K = Number of neighbors
 - Next K values: the id:s of the K neighbors
 - Next K values: the distances to the K neighbors

Example:
5000
0 5 24 99 483 11 444 1414008.500000 1238420.750000 0.000000 0.000000 0.000000
...
N=5000. First node id = 0, K = 5, neighbors = [24 99 483 11 444], distances = [1414008.500000 1238420.750000 .... ]

wgraph (graph format for text data only):
Each string and its neighbours in one line. Distance in brackets. Example:
neighbor: neighboor(1.0000), neighbour(1.0000), neibor(2.0000), neigbour(2.0000), neighbours(2.0000), neibour(3.0000), neither(3.0000)

=============
Run examples:

./rknng  --help

Run kNN-graph construction for birkbeckU.txt dataset (text data), store output to birkbeckU.knn:
./rknng  data/birkbeckU.txt  -o tmp/birkbeckU.knn  --type txt -k 20


Run RPDIV (faster) algorithm for string data birkbeckU.txt dataset, ground truth file (birkbeckU_k100_lev.gt) as parameter:
./rknng  data/birkbeckU.txt  -o tmp/birkbeck_test.knn --delta 0.01 --outf txt --gt data/birkbeckU_k100_lev.gt --algo rpdiv --type txt -k 20

Run NNDES algorithm for birkbeckU.txt dataset, output in ivec format:
./rknng  data/birkbeckU.txt  -o tmp/birkbeck_test_ivec.knn --delta 0.01 --outf txt --gt data/birkbeckU_k100_lev.gt --algo nndes --type txt -k 20

Run RPDIV algorithm for vectorial data/g2-256-50.txt (N=2048,D=256) dataset:
./rknng data/g2-256-50.txt -o tmp/g2-256-50.knn --outf txt --type vec


==========
References

[1] S. Sieranoja and P. Fränti. Fast random pair divisive construction of kNN graph using generic distance measures. In Proceedings of the 2018 International Conference on Big Data and Computing, ACM, 2018.
[2] W. Dong, C. Moses, and K. Li. Efficient k-nearest neighbor graph construction for generic similarity measures. In Proceedings of the 20th international conference on World wide web, page 577–586. ACM, 2011.

