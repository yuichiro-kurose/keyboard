// author: Yuichiro Kurose

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

const int kNumAlphabet = 26;
const int kNumHandKeys = 13;
const int kNumHandPositions = 15;
const int kNumFingers = 4;

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
  std::string line;
  while (std::cin >> line) {
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

// --- Evaluation Logic ---

struct KeyInfo {
  int hand_id;   // 0: Left, 1: Right
  int finger_id; // 0: Index, 1: Middle, 2: Ring, 3: Pinky
  int cost;
  bool valid;    // True if a letter is assigned
};

int main() {
  // 1. Load Document
  std::cerr << "Enter Document Text (end with 'END'):" << std::endl;
  std::string doc_text = LoadDoc();

  if (doc_text.empty()) {
    std::cerr << "Error: Document is empty." << std::endl;
    return 1;
  }

  // 2. Load Layout Configuration
  // Input format expects 15 tokens for Left Hand, then 15 tokens for Right Hand.
  // Use '_' for empty keys.
  // This matches the visual output format of Generator.cpp if copied linearly.
  std::vector<KeyInfo> char_map(kNumAlphabet, {-1, 0, -1, false});

  std::cerr << "Enter Hand 0 (Left) Layout (15 keys, row by row):" << std::endl;
  for (int i = 0; i < kNumHandPositions; ++i) {
    std::string token;
    std::cin >> token;
    if (token.length() == 1 && 'a' <= token[0] && token[0] <= 'z') {
      int char_idx = token[0] - 'a';
      int finger_idx = kHandLayout[i].finger_id_;
      int cost = kHandLayout[i].cost_group_;
      char_map[char_idx] = {0, finger_idx, cost, true};
    }
  }

  std::cerr << "Enter Hand 1 (Right) Layout (15 keys, row by row):" << std::endl;
  for (int i = 0; i < kNumHandPositions; ++i) {
    std::string token;
    std::cin >> token;

    int row = i / 5;
    int col = i % 5;
    int logical_idx = (row + 1) * 5 - col - 1;

    if (token.length() == 1 && 'a' <= token[0] && token[0] <= 'z') {
      int char_idx = token[0] - 'a';
      int finger_idx = kHandLayout[logical_idx].finger_id_;
      int cost = kHandLayout[i].cost_group_;
      char_map[char_idx] = {1, finger_idx, cost, true};
    }
  }

  // 3. Calculate Metrics

  // A: Calculate Finger Cost using Generator's definition (Static Graph Weight)
  std::vector<std::vector<int>> bigram_weights = BuildBigramWeights(doc_text);

  int hand_continuity_cost = 0;
  int finger_continuity_cost = 0;

  for (int i = 0; i < kNumAlphabet; i++) {
    for (int j = i; j < kNumAlphabet; j++) {
      if (char_map[i].hand_id == char_map[j].hand_id) {
        hand_continuity_cost += bigram_weights[i][j];
        if (char_map[i].finger_id == char_map[j].finger_id) {
          finger_continuity_cost += bigram_weights[i][j];
        }
      }
    }
  }

  // B: Calculate Dynamic metrics by iterating document

  long long same_hand_consecutive_count = 0;
  long long same_finger_consecutive_count = 0;

  for (size_t i = 0; i < doc_text.length() - 1; ++i) {
    int c1 = doc_text[i] - 'a';
    int c2 = doc_text[i+1] - 'a';

    // Skip if character not in layout (e.g. somehow missed or error)
    if (!char_map[c1].valid || !char_map[c2].valid) continue;

    // Metric 1: One-hand consecutive usage
    if (char_map[c1].hand_id == char_map[c2].hand_id) {
      same_hand_consecutive_count++;
    }

    // Metric 3: Same finger consecutive usage (Raw Count)
    // Note: Since assignments are unique to fingers (finger implies hand),
    // we just check finger_id. However, fingers are 0-3 per hand.
    // We must check if they are on the SAME HAND and SAME FINGER.
    if (char_map[c1].hand_id == char_map[c2].hand_id && 
        char_map[c1].finger_id == char_map[c2].finger_id) {
      same_finger_consecutive_count++;
    }
  }

  long long cost = 0;

  for (size_t i = 0; i < doc_text.length(); ++i) {
    int c = doc_text[i] - 'a';
    cost += char_map[c].cost;
  }

  // 4. Output Results
  std::cout << "\n=== Evaluation Results ===" << std::endl;
  std::cout << "Target Document Length: " << doc_text.length() << " characters" << std::endl;
  std::cout << "1. Hand movement cost (Algorithm Definition): " << hand_continuity_cost << std::endl;
  std::cout << "2. One-hand consecutive usage count (Actual): " << same_hand_consecutive_count << std::endl;
  std::cout << "3. Finger movement cost (Algorithm Definition): " << finger_continuity_cost << std::endl;
  std::cout << "4. Same finger consecutive usage count (Actual): " << same_finger_consecutive_count << std::endl;
  std::cout << "5. Difficulty in pressing: " << cost << std::endl;
  
  return 0;
}
