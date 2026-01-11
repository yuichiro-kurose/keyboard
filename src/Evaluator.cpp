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
    bigram_weights[idx2][idx1]++;
  }
  return bigram_weights;
}

// @brief 指ごとの連続使用コスト（同じ指の文字ペアの重み総和）を計算する
int CalcFingerContinuityCost(
    const std::vector<std::vector<int>>& finger_letter_assignments,
    const std::vector<std::vector<int>>& bigram_weights) {
  int total_cost = 0;
  for (const auto& letters_for_this_finger : finger_letter_assignments) {
    for (size_t i = 0; i < letters_for_this_finger.size(); ++i) {
      for (size_t j = i + 1; j < letters_for_this_finger.size(); ++j) {
        int letter_idx1 = letters_for_this_finger[i];
        int letter_idx2 = letters_for_this_finger[j];
        total_cost += bigram_weights[letter_idx1][letter_idx2];
      }
    }
  }
  return total_cost;
}

// --- Evaluation Logic ---

struct KeyInfo {
  int hand_id;   // 0: Left, 1: Right
  int finger_id; // 0: Index, 1: Middle, 2: Ring, 3: Pinky
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
  std::vector<KeyInfo> char_map(kNumAlphabet, {-1, -1, false});
  std::vector<std::vector<int>> finger_letter_assignments(kNumFingers);

  std::cerr << "Enter Hand 0 (Left) Layout (15 keys, row by row):" << std::endl;
  for (int i = 0; i < kNumHandPositions; ++i) {
    std::string token;
    std::cin >> token;
    if (token.length() == 1 && 'a' <= token[0] && token[0] <= 'z') {
      int char_idx = token[0] - 'a';
      int finger_idx = kHandLayout[i].finger_id_;
      char_map[char_idx] = {0, finger_idx, true};
      finger_letter_assignments[finger_idx].push_back(char_idx);
    }
  }

  std::cerr << "Enter Hand 1 (Right) Layout (15 keys, row by row):" << std::endl;
  for (int i = 0; i < kNumHandPositions; ++i) {
    std::string token;
    std::cin >> token;
    // Generator.cpp outputs right hand mirrored in print logic, 
    // but the array index mapping follows kHandLayout logic (0-14).
    // Note: Generator.cpp's PrintHandLayout logic for Hand 1:
    // key_idx = (row + 1) * 5 - col - 1;
    // This means the user input (visual order) needs to be mapped back to logical index 0-14.
    // Visual Input: Row 0 (Cols 0-4), Row 1...
    
    // However, to keep input simple, we assume the user provides the visual layout strings.
    // We must reverse-engineer the visual position to the logical index for Hand 1.
    // Logical 0-4 = Row 0 (Right to Left visually? No, standard split keyboards usually mirror).
    // Let's look closely at Generator.cpp PrintHandLayout:
    // Hand 0: key_idx = row * 5 + col
    // Hand 1: key_idx = (row + 1) * 5 - col - 1
    // Visual Row 0, Col 0 (Top Left of Right Hand block) -> Row 0, Col 0 -> Index 4.
    // Visual Row 0, Col 4 (Top Right of Right Hand block) -> Row 0, Col 4 -> Index 0.
    // This implies Hand 1 indices fill form Right-to-Left (Mirror).
    
    // BUT, kHandLayout defines fingers for indices 0-14.
    // Index 0 is Pinky (Finger 3). 
    // If Visual Input is "p y f g c", 'p' is at Col 0. 
    // Is Col 0 Index 4 (Index Finger)? No, Index 0 is Pinky.
    // Let's check kHandLayout again. 
    // Index 0: Finger 3 (Pinky).
    // Index 4: Finger 0 (Index).
    // So for Hand 1:
    // If 'p' is at Visual Col 0 (Leftmost of Right hand block), it should be Index Finger?
    // Usually standard QWERTY: Right hand 'y' (Index) is left-most. 'p' (Pinky) is right-most.
    // Generator Output:
    // col 0..4.
    // key_idx formula: (row+1)*5 - col - 1.
    // Col 0 -> 5 - 0 - 1 = 4. (Index 4 is Finger 0/Index). Correct.
    // Col 4 -> 5 - 4 - 1 = 0. (Index 0 is Finger 3/Pinky). Correct.
    
    // Therefore: The user inputs visual order (Col 0 to 4).
    // We map that `visual_idx` to `logical_idx` using the formula.
    
    int row = i / 5;
    int col = i % 5;
    int logical_idx = (row + 1) * 5 - col - 1;

    if (token.length() == 1 && 'a' <= token[0] && token[0] <= 'z') {
      int char_idx = token[0] - 'a';
      int finger_idx = kHandLayout[logical_idx].finger_id_;
      char_map[char_idx] = {1, finger_idx, true};
      finger_letter_assignments[finger_idx].push_back(char_idx);
    }
  }

  // 3. Calculate Metrics

  long long same_hand_consecutive_count = 0;
  long long same_finger_consecutive_count = 0;
  // Note: "Finger movement cost" as per Generator definition is sum of Bigram Weights
  // for letters on the same finger.
  
  // A: Calculate Finger Cost using Generator's definition (Static Graph Weight)
  std::vector<std::vector<int>> bigram_weights = BuildBigramWeights(doc_text);
  int finger_continuity_cost = CalcFingerContinuityCost(finger_letter_assignments, bigram_weights);

  // B: Calculate Dynamic metrics by iterating document
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

  // 4. Output Results
  std::cout << "\n=== Evaluation Results ===" << std::endl;
  std::cout << "Target Document Length: " << doc_text.length() << " characters" << std::endl;
  std::cout << "1. One-hand consecutive usage count: " << same_hand_consecutive_count << std::endl;
  std::cout << "2. Finger movement cost (Algorithm Definition): " << finger_continuity_cost << std::endl;
  std::cout << "3. Same finger consecutive usage count (Actual): " << same_finger_consecutive_count << std::endl;
  
  return 0;
}
