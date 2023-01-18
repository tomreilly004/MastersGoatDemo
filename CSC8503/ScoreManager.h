#pragma once

namespace NCL {
	namespace CSC8503 {
		class ScoreManager {
		public:
			ScoreManager() {
				score = 0;
			}
			~ScoreManager();

			void UpdateScore() {
				score += 5;
			}

			int GetScore() {
				return score;
			}

			void ResetScore() {
				score = 0;
			}

		protected:
			int score;
		};
	}
}

