#include <iostream>
#include <string.h>
#include <algorithm>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#define int_max 10000000
#define int_min -10000000
#define min(a, b) a<b?a:b
#define max(a, b) a>b?a:b
using namespace std;

struct Point {
	int x, y;
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};
enum SPOT_STATE { EMPTY = 0, BLACK = 1, WHITE = 2 };

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
std::vector<int>value;
Point x1 = Point(1, 1), x2 = Point(1, 6), x3 = Point(6, 1), x4 = Point(6, 6);
Point c1 = Point(0, 1), c2 = Point(0, 6), c3 = Point(1, 0), c4 = Point(1, 7), c5 = Point(6, 0), c6 = Point(6, 7), c7 = Point(7, 1), c8 = Point(7, 6);

#define myturn player
#define enemyturn (3-player)

void read_board(std::ifstream& fin) {
	fin >> player;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			fin >> board[i][j];
		}
	}
}

void read_valid_spots(std::ifstream& fin) {
	int n_valid_spots;
	fin >> n_valid_spots;
	int x, y;
	for (int i = 0; i < n_valid_spots; i++) {
		fin >> x >> y;
		next_valid_spots.push_back({ x, y });
	}
}
class Gamestate {
private:
	int get_next_player(int player)const {
		return 3 - player;
	}
	bool is_spot_on_board(Point p)const {
		return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
	}
	int get_disc(Point p)const {
		return gameboard[p.x][p.y];
	}
	void set_disc(Point p, int disc) {
		gameboard[p.x][p.y] = disc;
	}
	bool is_disc_at(Point p, int disc)const {
		if (!is_spot_on_board(p))return false;
		if (get_disc(p) != disc)return false;
		return true;
	}
	bool is_spot_valid(Point center)const {
		if (get_disc(center) != EMPTY)return false;
		for (Point dir : directions) {
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))continue;
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player))return true;
				p = p + dir;
			}
		}
		return false;
	}
	void flip_discs(Point center) {
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			std::vector<Point> discs({ p });
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player)) {
					for (Point s : discs) {
						set_disc(s, cur_player);
					}
					disc_count[cur_player] += discs.size();
					disc_count[get_next_player(cur_player)] -= discs.size();
					break;
				}
				discs.push_back(p);
				p = p + dir;
			}
		}
	}
public:
	const std::array<Point, 8> directions{ {
		Point(-1, -1), Point(-1, 0), Point(-1, 1),
		Point(0, -1), /*{0, 0}, */Point(0, 1),
		Point(1, -1), Point(1, 0), Point(1, 1)
	} };
	std::array<std::array<int, SIZE>, SIZE> gameboard;
	std::vector<Point> next_valid_spts;
	std::array<int, 3> disc_count;
	int cur_player;
	bool done;
	int winner;
	int depth;
	int alpha, beta;
	int value;

	//basic constrctor
	Gamestate() {
		next_valid_spts = get_valid_spots();
	}
	//constructor
	Gamestate(array<array<int, SIZE>, SIZE>board, int cur_player, int depth, int alpha, int beta) {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				this->gameboard[i][j] = board[i][j];
			}
		}
		this->cur_player = cur_player;
		this->alpha = alpha;
		this->beta = beta;
		this->depth = depth;
		this->next_valid_spts = get_valid_spots();
	}
	std::vector<Point> get_valid_spots()const {
		std::vector<Point> valid_spots;
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				Point p = Point(i, j);
				if (gameboard[i][j] != EMPTY)
					continue;
				if (is_spot_valid(p))
					valid_spots.push_back(p);
			}
		}
		return valid_spots;
	}
	bool put_disc(Point p) {
		if (!is_spot_valid(p)) {
			winner = get_next_player(cur_player);
			done = true;
			return false;
		}
		set_disc(p, cur_player);
		disc_count[cur_player]++;
		disc_count[EMPTY]--;
		flip_discs(p);
		// Give control to the other player.
		cur_player = get_next_player(cur_player);
		next_valid_spots = get_valid_spots();
		// Check Win
		if (next_valid_spots.size() == 0) {
			cur_player = get_next_player(cur_player);
			next_valid_spots = get_valid_spots();
			if (next_valid_spots.size() == 0) {
				// Game ends
				done = true;
				int white_discs = disc_count[WHITE];
				int black_discs = disc_count[BLACK];
				if (white_discs == black_discs) winner = EMPTY;
				else if (black_discs > white_discs) winner = BLACK;
				else winner = WHITE;
			}
		}
		return true;
	}
	int evaluation(Point p) {
		int eval = next_valid_spts.size();
		if (p == x1 || p == x2 || p == x3 || p == x4)eval += -20;
		if (p == c1 || p == c2 || p == c3 || p == c4 || p == c5 || p == c6 || p == c7 || p == c8)eval += -10;
		return eval;
	}
	
	int alphabeta_pruning(int dep, int now_player, int alpha, int beta, Point p) {
		if (dep >= 5)
			return evaluation(p);
		else {
			if (now_player == myturn) {
			//	cout << "Point p = " << p.x << ' ' << p.y << endl;
				Gamestate tempgame = Gamestate(gameboard, now_player, dep, alpha, beta);
				tempgame.put_disc(p);
			/*	for (int i = 0; i < SIZE; i++) {
					for (int j = 0; j < SIZE; j++) {
						if (tempgame.gameboard[i][j] == EMPTY)cout << '-';
						else if (tempgame.gameboard[i][j] == BLACK)cout << 'O';
						else if (tempgame.gameboard[i][j] == WHITE)cout << 'X';
					}
					cout << endl;
				}*/
				tempgame.next_valid_spts = tempgame.get_valid_spots();
			//	cout << tempgame.next_valid_spts.size() << endl;
				for (Point tp : tempgame.next_valid_spts) {
					int value = -tempgame.alphabeta_pruning(dep + 1, get_next_player(now_player), alpha, beta, tp);
			//		cout << "depth=" << dep + 1 << ", player=" << now_player << ", value=" << value << endl;
					if (value > alpha)alpha = value;
					if (alpha >= beta)break;
				}
			//	cout << "alpha=" << alpha << endl;
				return alpha;		
			}
			else if (now_player == enemyturn) {
			//	cout << "Point p = " << p.x << ' ' << p.y << endl;
				Gamestate tempgame = Gamestate(gameboard, now_player, dep, alpha, beta);
				tempgame.put_disc(p);
				for (int i = 0; i < SIZE; i++) {
					for (int j = 0; j < SIZE; j++) {
						if (tempgame.gameboard[i][j] == EMPTY)cout << '-';
						else if (tempgame.gameboard[i][j] == BLACK)cout << 'O';
						else if (tempgame.gameboard[i][j] == WHITE)cout << 'X';
					}
					cout << endl;
				}
				tempgame.next_valid_spts = tempgame.get_valid_spots();
			//	cout << tempgame.next_valid_spts.size() << endl;
				for (Point tp : tempgame.next_valid_spts) {
					int value = tempgame.alphabeta_pruning(dep + 1, get_next_player(now_player), alpha, beta, tp);
			//		cout << "depth=" << dep + 1 << ", player=" << now_player << ", value=" << value << endl;
					if (value < beta)
						beta = value;
					if (alpha >= beta)
						break;
				}
			//	cout << "beta=" << beta << endl;
				return beta;
			}
		}
	}
};
void write_valid_spot(std::ofstream& fout) {
	int n_valid_spots = next_valid_spots.size();
	srand(time(NULL));
	// Choose random spot. (Not random uniform here)
	Gamestate game(board, player, 0, int_min, int_max);
	vector<int>value;
	int max_index = 0;
	for (Point p : game.next_valid_spts) {
		int val = game.alphabeta_pruning(game.depth, game.cur_player, game.alpha, game.beta, p);
		value.push_back(val);
		for (int i = 0; i < value.size(); i++) {
			if (value[max_index] < value[i])max_index = i;
		}
		Point p = game.next_valid_spts[max_index];
		// Remember to flush the output to ensure the last action is written to file.
		fout << p.x << " " << p.y << std::endl;
		fout.flush();
	}
	
}

int main(int, char** argv) {
	std::ifstream fin(argv[1]);
	std::ofstream fout(argv[2]);
	read_board(fin);
	read_valid_spots(fin);
	write_valid_spot(fout);
	fin.close();
	fout.close();
/*	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			board[i][j] = EMPTY;
		}
	}*/
/*	board[3][4] = board[4][3] = board[3][3] = board[2][3] = BLACK;
	board[4][4] = WHITE;*/
/*	board[3][4] = board[4][3] = BLACK;
	board[3][3] = board[4][4] = WHITE;*/

/*	player = WHITE;
	Gamestate game(board, player, 0, int_min, int_max);
	vector<int>value;
	for (Point p : game.next_valid_spts) {
		int val = game.alphabeta_pruning(game.depth, game.cur_player, game.alpha, game.beta, p);
		cout << "true value=" << val << endl;
		value.push_back(val);
	}
	int max_index = 0;
	for (int i = 0; i < value.size(); i++) {
		if (value[max_index] < value[i])max_index = i;
	}
	cout << max_index << endl;
	for (Point p : game.next_valid_spts) {
		cout << p.x << ' ' << p.y << endl;
	}*/
	return 0;
}
