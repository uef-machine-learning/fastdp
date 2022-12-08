
#include "dataset.hpp"


float dice_set_distance(DataSet *sd, int a, int b) {
  float d =
      (float)(1.0 - dice_ngram(sd->bigrams[a], sd->bigrams[b], sd->setSize[a], sd->setSize[b]));
  // printf("type=%d d(%d,%d)=%f\n",sd->type,a,b,d);
  return d;
}

// Copied from
// https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf/6089413#6089413
std::istream &safeGetline(std::istream &is, std::string &t) {
  t.clear();

  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.

  std::istream::sentry se(is, true);
  std::streambuf *sb = is.rdbuf();

  for (;;) {
    int c = sb->sbumpc();
    switch (c) {
    case '\n':
      return is;
    case '\r':
      if (sb->sgetc() == '\n')
        sb->sbumpc();
      return is;
    case std::streambuf::traits_type::eof():
      // Also handle the case when the last line has no line ending
      if (t.empty())
        is.setstate(std::ios::eofbit);
      return is;
    default:
      t += (char)c;
    }
  }
}

DataSet *loadSetData(std::string infname) {
  std::string line;
  std::string delim = " ";
  std::ifstream infile(infname);
  DataSet *sd = (DataSet *)malloc(sizeof(DataSet));
  sd->strings = new vector<string>;
  sd->type = T_SET; // String data
  int numLines = 0;
  int numItems = 0;
  int buf[1000000];

  std::map<std::string, int> m;
  std::stringstream ss(line);
  std::string item;
  int id = -1;
  sd->size = 0;
  while (std::getline(infile, line)) {
    sd->size++;
  }
  infile.clear();
  infile.seekg(0, ios::beg);
  printf("Num lines: %d\n", sd->size);

  sd->bigrams = (int **)malloc(sizeof(int *) * sd->size);
  sd->setSize = (int *)malloc(sizeof(int) * sd->size);

  // while (std::getline(infile, line)) {
  int show_sets = 1;
  while (safeGetline(infile, line)) {
    if (numLines >= (sd->size)) {
      break;
      // safeGetline seems to produce one empty line that is between last \n and EOF
    }
    sd->strings->push_back(line);
    if (show_sets) {
      cout << "str: " << line << endl;
    }
    std::stringstream ss(line);
    std::string item;
    int id = -1;
    int setSize = 0;
    while (std::getline(ss, item, ' ')) {
      if (item.size() < 1) {
        continue;
      }
      if (m.find(item) == m.end()) {
        m[item] = numItems;
        numItems++;
      } else {
      }
      id = m[item];
      if (show_sets) {
        cout << "  word: '" << item << "' mapping:" << id << endl;
      }
      buf[setSize] = id;
      setSize++;
    }
    if (show_sets) {

      cout << "ids: ";
    }
    sd->bigrams[numLines] = (int *)malloc(sizeof(int) * setSize);
    sd->setSize[numLines] = setSize;
    for (int i = 0; i < setSize; i++) {
      sd->bigrams[numLines][i] = buf[i];
      if (show_sets) {

        cout << sd->bigrams[numLines][i] << " ";
      }
    }
    if (show_sets) {

      cout << endl;
    }

    numLines++;
    if (numLines > 5) {
      show_sets = 0;
    }
    if (numLines == (sd->size - 1)) {
      show_sets = 1;
    }
  }

  cout << "============" << endl;
  cout << flush;
  printf("Number of objects: %d\n", numLines);

  return sd;
}

DataSet *init_DataSet(int size, int dimensionality) {

  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));
  DS->size = size;
  DS->dimensionality = dimensionality;
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);
  DS->type = 1; // numerical
  float **datap = DS->data;
  for (int i = 0; i < DS->size; i++) {
    *datap = (float *)safemalloc(sizeof(float) * DS->dimensionality);
    datap++;
  }

  return DS;
}

void free_DataSet(DataSet *DS) {

  for (int i = 0; i < DS->size; i++) {
    if (DS->type == 1) { // Numerical
      free(DS->data[i]);
    }
  }
  free(DS->data);
  free(DS);
}

float *get_vector(DataSet *DS, int idx) { return (float *)*(DS->data + idx); }

void set_val(DataSet *DS, int data_i, int elem_i, float val) {
  *(*(DS->data + data_i) + elem_i) = val;
}

float get_val(DataSet *DS, int data_i, int elem_i) {
  return *(*(DS->data + data_i) + elem_i);
  /*return DS->data[data_i][elem_i]; //TODO*/
}

int dump_vector(DataSet *DS, int idx) {

  float *v = get_vector(DS, idx);

  for (int i = 0; i < DS->dimensionality; i++) {
    printf(" %e", *v);
    v++;
  }
  printf("\n");

  return 0;
}

float minkowskiDist(float *p1_idx, float *p2_idx, int D, float p) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += pow(abs(tmp), p);
    p1_idx++;
    p2_idx++;
  }
  // TODO: take 1/p root? No effect on knn search, only scaling.
  /*printf("dist:%f\n",dist);*/

  return dist;
}

// L0.5 distance, without final 1/p root
float L05dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    /*dist += tmp * tmp;*/
    dist += pow(tmp, 0.5);
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

// L1 distance
float L1dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    /*dist += tmp * tmp;*/
    dist += abs(tmp);
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

float L2dist(float *p1_idx, float *p2_idx, int D) {

  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < D; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += tmp * tmp;
    p1_idx++;
    p2_idx++;
  }
  // TODO: sqrt?

  return dist;
  /*return sqrt(dist);*/
}

float cosine_dist(float *p1_idx, float *p2_idx, int D) {
  float dist = 0;
  float a_sum = 0;
  float b_sum = 0;
  float product = 0;
  for (int i = 0; i < D; i++) {
    product += (*p1_idx) * (*p2_idx);
    a_sum += (*p1_idx) * (*p1_idx);
    b_sum += (*p2_idx) * (*p2_idx);
    p1_idx++;
    p2_idx++;
  }
  dist = 1 - product * 1.0 / (sqrt(a_sum) * sqrt(b_sum));
  return dist;
}

float distance(DataSet *P, int p1, int p2) {
  float ret;

  if (P->type == 2) // String data
  {
    if (g_options.distance_type == 10) {
      ret = dice_distance_precalc(P, p1, p2);
    } else {
      int distr = edit_distance(P->strings->at(p1), P->strings->at(p2));
      ret = (float)distr;
    }
    return ret;
  } else if (P->type == T_SET) {
    ret = dice_set_distance(P, p1, p2);
    return ret;
  } else { // Numerical data

    float *p1_idx = get_vector(P, p1);
    float *p2_idx = get_vector(P, p2);

    switch (g_options.distance_type) {
    case 0:
      ret = L2dist(p1_idx, p2_idx, P->dimensionality);
      break;
    case 1:
      ret = minkowskiDist(p1_idx, p2_idx, P->dimensionality, g_options.minkowski_p);
      break;
    case 2:
      ret = cosine_dist(p1_idx, p2_idx, P->dimensionality);
      break;

    default:
      exit(1); // TODO
    }

    return ret;
  }
}

DataSet *read_lshkit(const char *fname) {

  // From http://lshkit.sourceforge.net/dc/d46/matrix_8h.html :
  // " In the beginning of the file are three 32-bit integers: ELEM-SIZE, SIZE, DIM. ELEM-SIZE is
  // the size of the element, and currently the only valid value is 4, which is the size of float.
  // SIZE is the number of vectors in the file and DIM is the dimension. The rest of the file is
  // SIZE vectors stored consecutively, and the total size is SIZE * DIM * 4 bytes."

  int header[3];
  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));

  ifstream infile(fname, ios::binary);
  infile.read((char *)header, sizeof(int) * 3);
  if (header[0] != sizeof(float)) {
    printf("%d %zu", header[0], sizeof(float));
    terminal_error("Error reading input file. Format unknown.");
  }
  printf("Reading file with %d vectors of dimensionality %d\n", header[1], header[2]);

  DS->size = header[1];
  DS->dimensionality = header[2];
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);
  float **datap = DS->data;
  for (int i = 0; i < DS->size; i++) {
    /**datap = (float*) safemalloc(sizeof(float)*DS->dimensionality);*/
    // https://linux.die.net/man/3/memalign
    // TODO
    /**datap = (float*)memalign(MEM_ALIGNMENT, DS->dimensionality * sizeof(float));*/
    *datap = (float *)malloc(DS->dimensionality * sizeof(float));
    if (!*datap) {
      printf("mem align failed: i=%d\n", i);
      exit(1);
    }
    infile.read((char *)(*datap), DS->vector_size);
    datap++;
  }
  printf("Done reading.\n");

  return DS;
}

DataSet *read_ascii_dataset(const char *fname) {

  int N = 0;
  float buf;
  FILE *fp;
  int max_chars = 100000;
  char line[max_chars + 1];
  char *pbuf;
  int i_elem = 0;
  int dim = 0;
  printf("Reading ascii dataset file %s\n", fname);
  fp = fopen(fname, "r");
  if (!fp) {
    terminal_error("File does not exist\n");
  }

  N = count_lines(fp);
  printf("lines=%d\n", N);

  // Get number of elements
  char *ok = fgets(line, max_chars, fp);
  if (ok == NULL) {
    terminal_error("");
  }
  pbuf = line;
  for (i_elem = 0;; i_elem++) {
    if (*pbuf == '\n')
      break;
    buf = strtof(pbuf, &pbuf);
    printf(" %f", buf);
  }
  dim = i_elem;
  printf("\nnum_vectors=%d D=%d\n", N, dim);

  DataSet *DS = (DataSet *)safemalloc(sizeof(DataSet));
  DS->size = N;
  DS->dimensionality = dim;
  DS->vector_size = DS->dimensionality * sizeof(float);
  DS->data = (float **)safemalloc(sizeof(float *) * DS->size);

  fseek(fp, 0L, SEEK_SET);
  float **datap = DS->data;
  for (int i_vector = 0; i_vector < N; i_vector++) {
    // TODO: mem alignment needed?
    /**datap = (float*)memalign(MEM_ALIGNMENT, DS->dimensionality * sizeof(float));*/
    *datap = (float *)malloc(DS->dimensionality * sizeof(float));
    if (!*datap) {
      printf("mem align failed: i=%d\n", i_vector);
      exit(1);
    }

    float *col_p = *datap;

    char *ok = fgets(line, max_chars, fp);
    if (ok == NULL) {
      terminal_error("premature end of file");
    }
    /*printf("|%s |\n",line);*/
    pbuf = line;

    for (i_elem = 0;; i_elem++) {
      if (*pbuf == '\n' && i_elem == dim) {
        break;
      } else if (*pbuf == '\n') {
        terminal_error("Got too few elements");
      }
      buf = strtof(pbuf, &pbuf);
      if (i_elem <= dim) {
        *col_p = buf;
        /*if(i_vector==0) {printf("%f ",buf);}*/
      } else {
        terminal_error("");
      }
      col_p++;
    }

    datap++;
  }

  // DEBUG:
  /*for(int i=0;i<3;i++) {*/
  /*for(int j=0;j<DS->dimensionality;j++) {*/
  /*printf("%f ",get_val(DS, i, j));*/
  /*}*/
  /*printf("\n");*/
  /*}*/

  return DS;
}

// Rest of this file for string data

// Dice distance function based on:
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Dice%27s_coefficient#C

/* unsafe macro hashes bigrams into single int */
#define HASH(x, y) (((int)(x) << 16) | (y))
#define HASHtg(x, y, z) (((int)(x) << 16) | (y) | ((int)(z) << 8))

/* comparison function for qsort */
int cmp(const void *a, const void *b) {
  int x = *(int *)a, y = *(int *)b;
  return x - y;
}

using namespace std;

DataSet *loadStringData(std::string infname) {
  std::string line;
  std::ifstream infile(infname);
  DataSet *sd = (DataSet *)malloc(sizeof(DataSet));
  sd->strings = new vector<string>;
  sd->type = 2; // String data
  int numLines = 0;

  while (std::getline(infile, line)) {
    sd->strings->push_back(line);
    numLines++;
  }
  sd->size = numLines;

  sd->bigrams = (int **)malloc(sizeof(int *) * sd->size);
  sd->trigrams = (int **)malloc(sizeof(int *) * sd->size);
  for (int i = 0; i < sd->size; i++) {
    sd->bigrams[i] = calc_ngrams(sd->strings->at(i).c_str(), 2);
    sd->trigrams[i] = calc_ngrams(sd->strings->at(i).c_str(), 3);
  }

  return sd;
}

int *calc_ngrams(const char *str1, int ngram) {
  int *bg1;
  size_t i, strlen1, strlen2, setsize1, setsize2;

  strlen1 = strlen(str1);
  if (ngram == 1) {
    setsize1 = strlen1 - 1;
  } else if (ngram == 2) {
    setsize1 = strlen1 - 2;
  }

  /* allocate memory for bigram sets */
  setsize1 = strlen1 - 1;
  bg1 = (int *)calloc(setsize1, sizeof(int));
  if (!bg1) {
    //    return -1; TODO
  }

  if (ngram == 2) {
    /* hash the strings to produce bigram sets */
    for (i = 0; i < setsize1; i++) {
      bg1[i] = HASH(str1[i], str1[i + 1]);
    }
  } else if (ngram == 3) {

    /* hash the strings to produce bigram sets */
    for (i = 0; i < setsize1; i++) {
      bg1[i] = HASHtg(str1[i], str1[i + 1], str1[i + 2]);
    }
  }

  /* sort sets for ease of comparison */
  qsort(bg1, setsize1, sizeof(int), cmp);
  return bg1;
}

double dice_ngram(int *bg1, int *bg2, int setsize1, int setsize2) {
  //  int *bg1, *bg2;
  double matches;
  if (setsize1 < 1 || setsize2 < 1) {
    return 0.0;
  }

  /* compute the size of the intersection of bigram sets */
  matches = 0;
  for (size_t i = 0, j = 0; i < setsize1 && j < setsize2;) {
    if (bg1[i] == bg2[j]) {
      matches++;
      i++;
      j++;
    } else if (bg1[i] < bg2[j]) {
      i++;
    } else {
      j++;
    }
  }

  /* compute dice */
  return (2 * matches) / (setsize1 + setsize2);
}

float dice_distance_precalc(DataSet *sd, int a, int b) {

  int strlen1 = sd->strings->at(a).length();
  int strlen2 = sd->strings->at(b).length();
  float bigram =
      (float)(1.0 - dice_ngram(sd->bigrams[a], sd->bigrams[b], strlen1 - 1, strlen2 - 1));
  float trigram =
      (float)(1.0 - dice_ngram(sd->trigrams[a], sd->trigrams[b], strlen1 - 2, strlen2 - 2));

  float ret = trigram * 0.9 + bigram * 0.1;
  return ret;
}

double dice(const char *str1, const char *str2, int ngram) {
  int *bg1, *bg2;
  double matches;
  size_t i, strlen1, strlen2, setsize1, setsize2;

  /* make sure we've been given strings to compare and that they point to */
  /* two distinct places in memory                                        */
  if (str1 == NULL || str2 == NULL) {
    return 0;
  }
  if (str1 == str2) {
    return 1;
  }

  /* make sure strings are long enough (must have at least two chars) */
  strlen1 = strlen(str1);
  strlen2 = strlen(str2);
  if (strlen1 < 2 || strlen2 < 2) {
    return 0;
  }

  if (ngram == 2) {
    setsize1 = strlen1 - 1;
    setsize2 = strlen2 - 1;
  } else if (ngram == 3) {
    setsize1 = strlen1 - 2;
    setsize2 = strlen2 - 2;
  }

  /* allocate memory for bigram sets */
  setsize1 = strlen1 - 1;
  bg1 = (int *)calloc(setsize1, sizeof(int));
  if (!bg1) {
    return -1;
  }

  setsize2 = strlen2 - 1;
  bg2 = (int *)calloc(setsize2, sizeof(int));
  if (!bg2) {
    free(bg1);
    return -1;
  }

  if (ngram == 2) {
    /* hash the strings to produce bigram sets */
    for (i = 0; i < setsize1; i++) {
      bg1[i] = HASH(str1[i], str1[i + 1]);
    }

    for (i = 0; i < setsize2; i++) {
      bg2[i] = HASH(str2[i], str2[i + 1]);
    }
  } else if (ngram == 3) {

    /* hash the strings to produce bigram sets */
    for (i = 0; i < setsize1; i++) {
      bg1[i] = HASHtg(str1[i], str1[i + 1], str1[i + 2]);
    }

    for (i = 0; i < setsize2; i++) {
      bg2[i] = HASHtg(str2[i], str2[i + 1], str2[i + 2]);
    }
  }

  /* sort sets for ease of comparison */
  qsort(bg1, setsize1, sizeof(int), cmp);
  qsort(bg2, setsize2, sizeof(int), cmp);

  /* compute the size of the intersection of bigram sets */
  matches = 0;
  for (size_t i = 0, j = 0; i < setsize1 && j < setsize2;) {
    if (bg1[i] == bg2[j]) {
      matches++;
      i++;
      j++;
    } else if (bg1[i] < bg2[j]) {
      i++;
    } else {
      j++;
    }
  }

  /* always remember to free your memory */
  free(bg1);
  free(bg2);

  /* compute dice */
  return (2 * matches) / (setsize1 + setsize2);
}

float dice_distance(const std::string &s1, const std::string &s2) {
  //    float ret = (float) (1.0-dice_match(s1.c_str(), s2.c_str()));
  //  float ret = (float)(1.0 - dice(s1.c_str(), s2.c_str()));
  float bigram = (float)(1.0 - dice(s1.c_str(), s2.c_str(), 2));
  float trigram = (float)(1.0 - dice(s1.c_str(), s2.c_str(), 3));
  float ret = trigram * 0.9 + bigram * 0.1;
  return ret;
}

// Implementation copied from:
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
unsigned int edit_distance(const std::string &s1, const std::string &s2) {
  /*std::cout <<"s1:" << s1 << " s2:" << s2 << "\n";*/
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i)
    d[i][0] = i;
  for (unsigned int i = 1; i <= len2; ++i)
    d[0][i] = i;

  for (unsigned int i = 1; i <= len1; ++i)
    for (unsigned int j = 1; j <= len2; ++j)
      // note that std::min({arg1, arg2, arg3}) works only in C++11,
      // for C++98 use std::min(std::min(arg1, arg2), arg3)
      d[i][j] = std::min(
          {d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
  return d[len1][len2];
}

void debugStringDataset(DataSet *DS) {
  for (int i = 0; i < 10 && i < DS->size; i++) {
    string stmp = DS->strings->at(i);
    printf("i=%d/%d: %s\n", i, DS->size, stmp.c_str());
  }
}
