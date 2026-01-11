// author: Yuichiro Kurose

#include <algorithm>
#include <climits>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <utility>
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

// @brief 文字の出現頻度順のリストを構築する
// @param doc_text 入力ドキュメント
// @return `result[i]` = i番目に出現頻度が高い文字のインデックス (0='a')
std::vector<int> BuildFrequencyRankedLetters(const std::string& doc_text) {
  std::vector<std::pair<int, int>> letter_frequencies(kNumAlphabet);
  for (int i = 0; i < kNumAlphabet; i++) {
    letter_frequencies[i] = std::make_pair(0, i);
  }
  for (char c : doc_text) {
    letter_frequencies[c - 'a'].first++;
  }
  std::sort(letter_frequencies.rbegin(), letter_frequencies.rend());

  std::vector<int> ranked_letters(kNumAlphabet);
  for (int i = 0; i < kNumAlphabet; i++) {
    ranked_letters[i] = letter_frequencies[i].second;
  }
  return ranked_letters;
}

// @brief 2つの文字が隣接して出現する回数（重み）のグラフを構築する
std::vector<std::vector<int>> BuildBigramWeights(const std::string& doc_text) {
  std::vector<std::vector<int>> bigram_weights(
      kNumAlphabet, std::vector<int>(kNumAlphabet, 0));
  for (int i = 0; i < (int)doc_text.size() - 1; i++) {
    int idx1 = doc_text[i] - 'a';
    int idx2 = doc_text[i + 1] - 'a';
    bigram_weights[idx1][idx2]++;
    if (idx1 != idx2) {
      bigram_weights[idx2][idx1]++;
    }
  }
  return bigram_weights;
}

// @brief 片手の連続使用コスト（同じ手の文字ペアの重み総和）を計算する
int CalcHandConflictCost(
    int hand_bitmask,
    const std::vector<std::vector<int>>& bigram_weights) {
  int total_cost = 0;
  for (int i = 0; i < kNumAlphabet; i++) {
    for (int j = i; j < kNumAlphabet; j++) {
      if (((hand_bitmask >> i) & 1) == ((hand_bitmask >> j) & 1)) {
        total_cost += bigram_weights[i][j];
      }
    }
  }
  return total_cost;
}

// @brief 左右の手で担当する文字を最適に分割する
// @return `result[0]`=左手, `result[1]`=右手の担当文字インデックスリスト
std::vector<std::vector<int>> AssignLettersToHands(
    const std::vector<std::vector<int>>& bigram_weights) {
  int min_conflict_cost = INT_MAX;
  int optimal_hand_bitmask = (1 << kNumHandKeys) - 1;
  int current_hand_bitmask = (1 << kNumHandKeys) - 1;

  while (current_hand_bitmask < (1 << kNumAlphabet)) {
    int current_cost = CalcHandConflictCost(current_hand_bitmask, bigram_weights);
    if (current_cost < min_conflict_cost) {
      min_conflict_cost = current_cost;
      optimal_hand_bitmask = current_hand_bitmask;
    }

    // Gosper's Hack
    int x = current_hand_bitmask & -current_hand_bitmask;
    int y = current_hand_bitmask + x;
    current_hand_bitmask = ((current_hand_bitmask & ~y) / x >> 1) | y;
  }

  std::vector<std::vector<int>> hand_assignments(2);
  for (int i = 0; i < kNumAlphabet; i++) {
    int hand_index = (optimal_hand_bitmask >> i) & 1;
    hand_assignments[hand_index].push_back(i);
  }
  return hand_assignments;
}

// @brief 指ごとの連続使用コスト（同じ指の文字ペアの重み総和）を計算する
int CalcFingerContinuityCost(
    const std::vector<std::vector<int>>& finger_letter_assignments,
    const std::vector<std::vector<int>>& bigram_weights) {
  int total_cost = 0;
  for (const auto& letters_for_this_finger : finger_letter_assignments) {
    for (size_t i = 0; i < letters_for_this_finger.size(); ++i) {
      for (size_t j = i; j < letters_for_this_finger.size(); ++j) {
        int letter_idx1 = letters_for_this_finger[i];
        int letter_idx2 = letters_for_this_finger[j];
        total_cost += bigram_weights[letter_idx1][letter_idx2];
      }
    }
  }
  return total_cost;
}

// @brief 最適なキーレイアウトをコンソールに出力する
void PrintHandLayout(int hand_index,
                     const std::vector<int>& best_layout) {
  std::cout << "--- Hand " << hand_index << " (Optimal Layout) ---" << std::endl;

  for (int row = 0; row < 3; ++row) {
    for (int col = 0; col < 5; ++col) {
      int key_idx = (hand_index == 0) ? (row * 5 + col)
                                      : ((row + 1) * 5 - col - 1);
      int letter_idx = best_layout[key_idx];

      if (letter_idx != -1) {
        std::cout << (char)('a' + letter_idx) << " ";
      } else {
        std::cout << '_' << " ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << "-------------------------------------------------" << std::endl;
}

// @brief 片手のキー配置を計算するためのヘルパークラス
class KeyPlacer {
public:
  KeyPlacer(int hand, const std::vector<int>& hand_letters,
            const std::vector<int>& letters_sorted_by_frequency,
            const std::vector<std::vector<int>>& bigram_weights,
            const std::vector<std::vector<int>>& keys_by_cost_group,
            const std::map<int, int>& key_to_finger_map)
      : hand_(hand),
        letters_sorted_by_frequency_(letters_sorted_by_frequency),
        bigram_weights_(bigram_weights),
        keys_by_cost_group_(keys_by_cost_group),
        key_to_finger_map_(key_to_finger_map),
        min_finger_cost_(INT_MAX),
        best_layout_(kNumHandPositions, -1) {
    current_hand_letters_bitmask_ = 0;
    for (int letter_idx : hand_letters) {
      current_hand_letters_bitmask_ |= (1 << letter_idx);
    }
  }

  // @brief 計算を実行し、最適解を返す
  std::vector<int> Solve() {
    FindBestLayoutRecursive(0, std::queue<int>());
    return best_layout_;
  }

  int GetMinCost() const { return min_finger_cost_; }

private:
  const int hand_;
  const std::vector<int>& letters_sorted_by_frequency_;
  const std::vector<std::vector<int>>& bigram_weights_;
  const std::vector<std::vector<int>>& keys_by_cost_group_;
  const std::map<int, int>& key_to_finger_map_;

  int current_hand_letters_bitmask_;
  int min_finger_cost_;
  std::vector<int> best_layout_;

  // @brief 順列を全探索する再帰関数
  // @param cost_group_index 現在処理中のコストグループ (0~4)
  // @param current_key_queue 割り当て優先度順にキーインデックスが積まれたキュー
  void FindBestLayoutRecursive(int cost_group_index,
                               std::queue<int> current_key_queue) {
    if (cost_group_index == kNumCostGroups) {
      EvaluateLayout(current_key_queue);
      return;
    }

    std::vector<int> current_group_keys = keys_by_cost_group_[cost_group_index];
    std::sort(current_group_keys.begin(), current_group_keys.end());

    do {
      std::queue<int> next_queue = current_key_queue;
      for (int key_idx : current_group_keys) {
        next_queue.push(key_idx);
      }
      FindBestLayoutRecursive(cost_group_index + 1, next_queue);
    } while (std::next_permutation(current_group_keys.begin(),
                                   current_group_keys.end()));
  }

  // @brief 確定したキー割り当て順 (順列) に基づいてレイアウトを評価する
  void EvaluateLayout(std::queue<int> key_queue_for_this_permutation) {
    std::vector<std::vector<int>> finger_assignments(kNumFingers);
    std::vector<int> current_layout(kNumHandPositions, -1);

    for (int letter_idx : letters_sorted_by_frequency_) {
      if ((current_hand_letters_bitmask_ >> letter_idx) & 1) {
        int key_idx = key_queue_for_this_permutation.front();
        key_queue_for_this_permutation.pop();

        current_layout[key_idx] = letter_idx;
        int finger_id = key_to_finger_map_.at(key_idx);
        finger_assignments[finger_id].push_back(letter_idx);
      }
    }

    int current_finger_cost =
        CalcFingerContinuityCost(finger_assignments, bigram_weights_);

    if (current_finger_cost < min_finger_cost_) {
      min_finger_cost_ = current_finger_cost;
      best_layout_ = current_layout;
    }
  }
};

// @brief キーの配置を決定し、結果を出力する
void PlaceKeysOnHands(
    const std::vector<std::vector<int>>& hand_letter_assignments,
    const std::vector<int>& letters_sorted_by_frequency,
    const std::vector<std::vector<int>>& bigram_weights) {
  std::vector<std::vector<int>> keys_by_cost_group(kNumCostGroups);
  std::map<int, int> key_to_finger_map;

  for (const auto& key : kHandLayout) {
    keys_by_cost_group[key.cost_group_].push_back(key.key_index_);
    key_to_finger_map[key.key_index_] = key.finger_id_;
  }

  for (int hand = 0; hand < 2; hand++) {
    KeyPlacer placer(hand, hand_letter_assignments[hand],
                     letters_sorted_by_frequency, bigram_weights,
                     keys_by_cost_group, key_to_finger_map);

    std::vector<int> best_layout = placer.Solve();
    int min_finger_cost = placer.GetMinCost();

    PrintHandLayout(hand, best_layout);
  }
}

int main() {
  std::cerr << "Enter Document Text (end with 'END'):" << std::endl;
  std::string doc_text = LoadDoc();

  if (doc_text.empty()) {
    std::cerr << "Error: Document is empty." << std::endl;
    return 1;
  }
  
  std::vector<int> letters_sorted_by_frequency =
      BuildFrequencyRankedLetters(doc_text);
  std::vector<std::vector<int>> bigram_weights = BuildBigramWeights(doc_text);
  std::vector<std::vector<int>> hand_letter_assignments =
      AssignLettersToHands(bigram_weights);

  std::cout << "=== Splitting Keys ===" << std::endl;
  std::cout << "Hand 0 (Left) letters: ";
  for (int idx : hand_letter_assignments[0]) {
    std::cout << (char)('a' + idx) << " ";
  }
  std::cout << std::endl;
  std::cout << "Hand 1 (Right) letters: ";
  for (int idx : hand_letter_assignments[1]) {
    std::cout << (char)('a' + idx) << " ";
  }
  std::cout << "\n" << std::endl;

  std::cout << "=== Placing Keys ===" << std::endl;
  PlaceKeysOnHands(hand_letter_assignments, letters_sorted_by_frequency,
                   bigram_weights);

  return 0;
}
