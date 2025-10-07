// author: Yuichiro Kurose

#include <algorithm>
#include <climits>
#include <iostream>
#include <string>
#include <vector>

// ドキュメントを読み込む関数
std::string LoadDoc() {
  std::string doc = "";
  while (true) {
    std::string s;
    std::cin >> s;
    if (s == "END") {
      break;
    }
    for (char c : s) {
      if ('A' <= c && c <= 'z') {
        doc += tolower(c);
      }
    }
  }
  return doc;
}

// 頂点集合を{a,b,...,z}とする重み付き完全グラフを構築する関数
std::vector<std::vector<int>> BuildWeight(std::string doc) {
  std::vector<std::vector<int>> weight(26, std::vector<int>(26, 0));
  for (int i = 0; i < (int) doc.size() - 1; i++) {
    int idx1 = doc[i] - 'a', idx2 = doc[i + 1] - 'a';
    weight[idx1][idx2]++;
    weight[idx2][idx1]++;
  }
  return weight;
}

// 片手の連続使用回数を求める関数
int CalcCount(int combination, std::vector<std::vector<int>> weight) {
  int count = 0;
  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < 26; j++) {
      if (((combination >> i) & 1) == ((combination >> j) & 1)) {
        count += weight[i][j];
      }
    }
  }
  return count;
}

// 右手と左手の担当キーを決定する関数
std::vector<std::vector<int>> SplitKey(std::vector<std::vector<int>> weight) {
  int min_count = INT_MAX;
  int combination = (1 << 13) - 1, best_combination = (1 << 13) - 1;
  // {0,1,...,26}に含まれるサイズ13の部分集合を列挙
  while (combination < 1 << 26) {
    // ここで組み合わせに対して処理をする
    int count = CalcCount(combination, weight);
    if (count < min_count) {
      min_count = count;
      best_combination = combination;
    }
    int x = combination & -combination, y = combination + x;
    combination = ((combination & ~y) / x >> 1) | y;
  }
  std::vector<std::vector<int>> keyboard(2);
  for (int i = 0; i < 26; i++) {
    keyboard[(best_combination >> i) & 1].push_back(i);
  }
  return keyboard;
}

// キーの配置を決定する関数
void Place(std::vector<std::vector<int>>& keyboard, std::string doc) {
  // finger[i] := indexがiのキーを担当する指の番号
  std::vector<int> finger = {4, 3, 2, 1, 1, 4, 3, 2, 1, 1, 4, 3, 2, 1, 1};
  // cost[i] := コストがiであるキーのindex
  std::vector<std::vector<int>> cost(5);
  cost[0] = {7, 8};
  cost[1] = {2, 5, 6};
  cost[2] = {1, 3, 9, 12, 13};
  cost[3] = {4, 10, 11};
  cost[4] = {0, 14};
  // コストの和が最小となるようなキーの配置を全通り試し、同じ指の連続使用回数が最も少ない配置を採用
  for (int hand = 0; hand < 2; hand++) {
    do {
      do {
        do {
          do {
            do {
              // ここに処理を書く
            } while (std::next_permutation(cost[4].begin(), cost[4].end()));
          } while (std::next_permutation(cost[3].begin(), cost[3].end()));
        } while (std::next_permutation(cost[2].begin(), cost[2].end()));
      } while (std::next_permutation(cost[1].begin(), cost[1].end()));
    } while (std::next_permutation(cost[0].begin(), cost[0].end()));
  }
}

int main() {
  std::string doc = LoadDoc();
  std::vector<std::vector<int>> weight = BuildWeight(doc);
  std::vector<std::vector<int>> keyboard = SplitKey(weight);
  Place(keyboard, doc);
  return 0;
}
