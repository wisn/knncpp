#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
using namespace std;

/*! Build an abstract data type for CSV record data */
struct Datum {
  int index;
  double x1, x2, x3, x4, x5;
  char y;

  void print() {
    cout << this->index << ' '
      << this->x1 << ' '
      << this->x2 << ' '
      << this->x3 << ' '
      << this->x4 << ' '
      << this->x5 << ' '
      << this->y << endl;
  }
};
typedef vector<Datum> Data;

/*! Build an abstract data type for Point */
struct Point {
  char label;
  double distance;
};

/*! Build an abstract data type for kNN */
struct kNN {
  int k;
  Data train;

  kNN(Data train, int k) {
    this->k = k;
    this->train = train;
  }

  double distance(Datum a, Datum b) {
    double ret = 0;
    ret += pow(a.x1 - b.x1, 2);
    ret += pow(a.x2 - b.x2, 2);
    ret += pow(a.x3 - b.x3, 2);
    ret += pow(a.x4 - b.x4, 2);
    ret += pow(a.x5 - b.x5, 2);

    return sqrt(ret);
  }

  char predictions(Datum test) {
    int train_size = this->train.size();
    
    vector<Point> points;
    for (int j = 0; j < train_size; j++) {
      Datum train = this->train[j];

      Point point;
      point.label = train.y;
      point.distance = this->distance(test, train);

      points.push_back(point);
    }

    auto comparator = [](Point x, Point y) {
      if (x.distance == y.distance)
        return x.label < y.label;

      return x.distance < y.distance;
    };

    sort(points.begin(), points.end(), comparator);

    int labels[4];
    fill(labels, labels + 4, 0);
    for (int i = 0; i < this->k; i++) {
      labels[points[i].label - '0'] += 1;
    }

    int label = 0;
    int best = labels[0];
    for (int i = 0; i < 4; i++) {
      if (labels[i] > best) {
        best = labels[i];
        label = i;
      }
    }

    return '0' + label;
  }
};

/*! Build some helpers */
Data readCSV(string filename);
Data randomnize(Data data);

/*! Starting the main function */
int main() {
  cout << "Reading CSV file for Data Train..." << endl;
  Data data_train = readCSV("data/DataTrain_Tugas3_AI.csv");

  cout << endl << "Reading CSV file for Data Test..." << endl;
  Data data_test = readCSV("data/DataTest_Tugas3_AI.csv");

  cout << endl;

  // Test the kNN predictions function with K = 5
  [&]() {
    cout << "Test kNN predictions function with K = 5" << endl;
    cout << '[';

    kNN knn (data_train, 5);
    for (int i = 0; i < (int) data_test.size(); i++) {
      if (i > 0) cout << ", ";

      Datum test = data_test[i];
      cout << knn.predictions(test);
    }

    cout << ']' << endl;
  }();

  cout << endl << "Doing validation. Please wait..." << endl;

  int K;
  // Finding the best K value
  [&]() {
    Data validation_data = randomnize(data_train);
    Data validation_train, validation_test;

    int n = data_train.size();
    int m = data_test.size();
    for (int i = 0; i < (n - m); i++)
      validation_train.push_back(validation_data[i]);

    for (int i = (n - m); i < n; i++)
      validation_test.push_back(validation_data[i]);

    int max_k = m;
    vector<pair<int, double>> accuracy (max_k);
    for (int k = 1; k <= max_k; k++) {
      kNN knn (validation_train, k);
      int match = 0;

      for (int i = 0; i < m; i++) {
        Datum test = validation_test[i];
        int predictions = knn.predictions(test);
        if (predictions == test.y)
          match += 1;
      }

      accuracy[k - 1] = {k, (match / (double) m) * 100};
    }

    cout << "Accuracy calculation:" << endl;
    cout << '[';
    for (int i = 0; i < max_k; i++) {
      if (i > 0) cout << ", ";

      cout << "(K = " << accuracy[i].first << ", " << accuracy[i].second << "%)";
    }

    cout << ']' << endl << endl;

    cout << "Retrieving the best K value..." << endl;
    cout << "K = ";

    int current_k = accuracy[0].first;
    double current_accuracy = accuracy[0].second;
    for (int i = 0; i < max_k; i++) {
      if (accuracy[i].second > current_accuracy) {
        current_accuracy = accuracy[i].second;
        current_k = accuracy[i].first;
      }
    }

    K = current_k;
    cout << K << " with " << current_accuracy << " percentile." << endl;
  }();

  cout << endl;

  // Retrieve the solution based on K value above
  [&]() {
    vector<char> labels;
    kNN knn (data_train, K);
    int n = data_test.size();
    for (int i = 0; i < n; i++) {
      Datum test = data_test[i];
      labels.push_back(knn.predictions(test));
    }

    cout << "Retrieved solution:" << endl;
    cout << '[';

    for (int i = 0; i < n; i++) {
      if (i > 0) cout << ", ";

      cout << labels[i];
    }

    cout << ']' << endl;

    ofstream csv;
    csv.open("TebakanTugas3.csv");

    csv << "Y\n";
    for (int i = 0; i < n; i++)
      csv << labels[i] << '\n';

    csv.close();

    cout << endl;
    cout << "Solution saved as TebakanTugas3.csv" << endl;
  }();

  cout << endl;
  cout << "Closing..." << endl;

  return 0;
}

Data readCSV(string filename) {
  ifstream csv;
  csv.open(filename);

  Data data;

  string row;
  bool header = true;
  while (getline(csv, row)) {
    string col;
    stringstream streamer (row);

    vector<string> cols;
    while (getline(streamer, col, ','))
      cols.push_back(col);

    if (!header) {
      Datum datum;
      datum.index = stoi(cols[0]);
      datum.x1 = stod(cols[1]);
      datum.x2 = stod(cols[2]);
      datum.x3 = stod(cols[3]);
      datum.x4 = stod(cols[4]);
      datum.x5 = stod(cols[5]);
      datum.y = cols[6][0];

      data.push_back(datum);
    }
    else {
      header = false;
    }
  }

  csv.close();
  return data;
}

Data randomnize(Data data) {
  int n = data.size();
  mt19937 randomnizer (random_device{}());
  
  vector<int> index (n);
  for (int i = 0; i < n; i++) index[i] = i;

  shuffle(index.begin(), index.end(), randomnizer);

  Data ret (n);
  for (int i = 0; i < n; i++) ret[i] = data[index[i]];

  return ret;
}

