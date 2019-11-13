# FastDP

The FastDP software implements the following fast variant of Density Peaks clustering algorithm:

Sieranoja, Sami, and Pasi Fränti. "Fast and General Density Peaks Clustering." Pattern Recognition Letters (2019). https://www.sciencedirect.com/science/article/pii/S0167865519303009


The software is provided with GNU Lesser General Public License, Version 3. https://www.gnu.org/licenses/lgpl.html. All files except those under data and contrib -folders are subject to this license. See LICENSE.txt.

## Compile

```
make clean
make -C cbevi
make -C rknng apitest
make dencl
```



## Use cases

### Numerical (vectorial) data
Cluster vectorial (numerical) S1 dataset to 15 clusters by first constructing kNN graph k=30 neighbors:
```
./denc --neighbors 30 --clusters 15  --type=vec --knng-delta=0.1 data/s1.txt --calcnbprop 1 --out-pa=data/output_s1.pa
```

Same as above, but also evaluate clustering based on ground truth (s1-label) by calculating CI and NMI values: 
```
./denc data/s1.txt --neighbors 30 --clusters 15  --type=vec --knng-delta=0.1 --gt-pa=data/s1-label.pa  --out-pa=data/output_s1.pa
```

### String data
For short strings (e.g. words of around 8 characters), edit distance works ok.

Download list of 466551 English words:
```
wget https://github.com/dwyl/english-words/raw/master/words.txt
```
Cluster them (command should run in around 30 minutes.):
```
./denc  --neighbors=30 --clusters=500  --type=txt --knng-delta=0.01 --dfunc=lev --out-pa words.pa words.txt
```
Take 20 samples from each cluster, show as html (requires Ruby 2.0+):
```
./show_text_clusters.rb words.txt words.pa > words.html
```

Example clusters in result:
```
Cluster 24: taurophobia taphephobia Onychophora antlophobia Judaeophobia ischocholia graphophonic Russophilism Turcophilism ...
Cluster 313: incorruptibly incompletable undecomposable comportable unsummonable compellable competible computerizable compossible comportance ...
Cluster 481: roodles robotize whooplike rootless howkit stormlike hoggie Goetae nookiest boomless toodle-oo blowziest hokiest crowdie ...
```

For a faster, but lower quality version, use dice distance and disable use of NNDES (command should run in around 4 minutes): 
```
./denc  --neighbors=30 --clusters=500  --type=txt --knng-delta=0.01 --dfunc=dice --out-pa words_dice.pa --knng-nndes 0.0 words.txt
```

Dice distance works also for long strings of around 140 characters.

### Categorical/set data, Dice distance
This feature is somewhat experimental and has not been tested thoroughly.
```
./denc  --neighbors=30 --clusters=2000  --type=set --knng-delta=0.05 --dfunc=dice --out-pa=set_data.pa --knng-nndes 0.0 set_data.txt
```

Input format for set_data.txt. One set per line, members of set separated by spaces:
```
A B D E H
B D E F  
C D E F G 
C D G H
...
```

## File formats

Line ends with "\n" characters. Files end with "\n" 

Output .pa -file contains header information and cluster labels for each input item.

## Dataset generation (MATLAB)

Create a 2D dataset with a maximum of 20 clusters. Save to file 'worms_88.txt'.
```
gen_2d_worms(88,20);
```

Create a 64D dataset with a maximum of 20 clusters. Save to file 'worms_1064.txt'.
```
gen_high_dim_worms(1064,20,64);
```

In -labels file first column is label of data point, second column: 2=noise, 1=normal.

