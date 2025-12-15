// author: Yuichiro Kurose

#include <iostream>
#include <string>
#include <vector>

const int kNumAlphabet = 26;
const int kNumHandKeys = 13;
const int kNumHandPositions = 15;
const int kNumFingers = 4;
const int kNumCostGroups = 5;

// @brief 片手の物理キー1つが持つ情報を表現する構造体
struct PhysicalKey {
  int key_index_;
  int finger_id_;
  int cost_group_;
};

// @brief 片手15キーの標準物理レイアウト定義
const std::vector<PhysicalKey> kHandLayout = {
    {.key_index_ = 0, .finger_id_ = 3, .cost_group_ = 4},
    {.key_index_ = 1, .finger_id_ = 2, .cost_group_ = 2},
    {.key_index_ = 2, .finger_id_ = 1, .cost_group_ = 1},
    {.key_index_ = 3, .finger_id_ = 0, .cost_group_ = 2},
    {.key_index_ = 4, .finger_id_ = 0, .cost_group_ = 3},
    {.key_index_ = 5, .finger_id_ = 3, .cost_group_ = 1},
    {.key_index_ = 6, .finger_id_ = 2, .cost_group_ = 1},
    {.key_index_ = 7, .finger_id_ = 1, .cost_group_ = 0},
    {.key_index_ = 8, .finger_id_ = 0, .cost_group_ = 0},
    {.key_index_ = 9, .finger_id_ = 0, .cost_group_ = 2},
    {.key_index_ = 10, .finger_id_ = 3, .cost_group_ = 3},
    {.key_index_ = 11, .finger_id_ = 2, .cost_group_ = 3},
    {.key_index_ = 12, .finger_id_ = 1, .cost_group_ = 2},
    {.key_index_ = 13, .finger_id_ = 0, .cost_group_ = 2},
    {.key_index_ = 14, .finger_id_ = 0, .cost_group_ = 4}};

// @brief ドキュメントを標準入力から読み込む
std::string LoadDoc() {
  std::string doc_text = "";
  while (true) {
    std::string line;
    std::cin >> line;
    if (line == "END") {
      break;
    }
    for (char c : line) {
      if ('A' <= c && c <= 'z') {
        doc_text += tolower(c);
      }
    }
  }
  return doc_text;
}

// @brief キー配列を標準入力から読み込む
std::vector<std::vector<int>> LoadLayout() {
  std::vector<std::vector<int>> layout(2, std::vector<int>(kNumHandPositions));
  for (int hand_index = 0; hand_index < 2; hand_index++) {
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 5; col++) {
        char c;
        std::cin >> c;
        int key_index = (hand_index == 0) ? (row * 5 + col)
                                          : ((row + 1) * 5 - col - 1);
        if (c != '_') {
          layout[hand_index][key_index] = (int)(c - 'a');
        } else {
          layout[hand_index][key_index] = -1;
        }
      }
    }
  }
  return layout;
}

// @brief 各アルファベットを担当する手を表すビットマスクを構築する
int BuildHandBitmask(std::vector<int> left_layout) {
  int hand_bitmask = 0;
  for (int key_index = 0; key_index < kNumHandPositions; key_index++) {
    if (left_layout[key_index] != -1) {
      hand_bitmask |= (1 << left_layout[key_index]);
    }
  }
  return hand_bitmask;
}

int main() {
  std::string doc_text = LoadDoc();
  std::vector<std::vector<int>> layout = LoadLayout();
  int hand_bitmask = BuildHandBitmask(layout[0]);
  return 0;
}
