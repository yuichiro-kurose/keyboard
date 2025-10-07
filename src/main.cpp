// author: Yuichiro Kurose

#include <algorithm>
#include <climits>
#include <iostream>
#include <queue>
#include <string>
#include <utility>
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

// 出現頻度の順を表す配列を構築する関数
std::vector<int> BuildRank(std::string doc) {
  std::vector<std::pair<int, int>> count(26);
  for (int i = 0; i < 26; i++) {
    count[i] = std::make_pair(0, i);
  }
  for (char c : doc) {
    count[c - '0'].first++;
  }
  std::sort(count.begin(), count.end());
  std::reverse(count.begin(), count.end());
  std::vector<int> rank(26);
  for (int i = 0; i < 26; i++) {
    rank[i] = count[i].second;
  }
  return rank;
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
    int hand = (best_combination >> 1) & 1;
    keyboard[hand].push_back(i);
  }
  return keyboard;
}

// キーの配置を決定する関数
void Place(std::vector<std::vector<int>>& keyboard, std::vector<int> rank) {
  // finger[i] := indexがiのキーを担当する指の番号
  std::vector<int> finger = {3, 2, 1, 0, 0, 3, 2, 1, 0, 0, 3, 2, 1, 0, 0};
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
              // queue内で最も優先度が高いkeyが常に先頭
              std::queue<int> priority;
              for (int i = 0; i < 4; i++) {
                for (int idx : cost[i]) {
                  priority.push(idx);
                }
              }
              // 今注目ている手のアルファベットのbit集合
              int key = 0;
              for (int idx : keyboard[hand]) {
                key |= (1 << idx);
              }
              // combination[i] := 指iが担当するkey
              std::vector<std::vector<int>> combination(5);
              for (int i = 0; i < 26; i++) {
                if ((key >> rank[i]) & 1) {
                  int idx = priority.front();
                  priority.pop();
                  combination[finger[idx]].push_back(rank[i]);
                }
              }
              // 2025年10月07日
              // このループ内における配置での、指ごとの担当アルファベットを表す配列が完成
              // あとは指の連続使用回数を数え、最小であったものを採用するプログラムを作成する
            } while (std::next_permutation(cost[4].begin(), cost[4].end()));
          } while (std::next_permutation(cost[3].begin(), cost[3].end()));
        } while (std::next_permutation(cost[2].begin(), cost[2].end()));
      } while (std::next_permutation(cost[1].begin(), cost[1].end()));
    } while (std::next_permutation(cost[0].begin(), cost[0].end()));
  }
}

int main() {
  std::string doc = LoadDoc();
  std::vector<int> rank = BuildRank(doc);
  std::vector<std::vector<int>> weight = BuildWeight(doc);
  std::vector<std::vector<int>> keyboard = SplitKey(weight);
  Place(keyboard, rank);
  return 0;
}
