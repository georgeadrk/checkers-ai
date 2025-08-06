// checkers.cpp

#include <iostream>
#include <vector>
using namespace std;

const int EMPTY = 0;
const int P1_MAN = 1;
const int P1_KING = 2;
const int P2_MAN = -1;
const int P2_KING = -2;

const int BOARD_SIZE = 8;

typedef vector<vector<int>> Board;

struct Move {
    vector<pair<int, int>> path;
    bool isCapture;

    Move(pair<int, int> start) {
        path.push_back(start);
        isCapture = false;
    }

    void addStep(pair<int, int> pos) {
        path.push_back(pos);
    }

    pair<int, int> from() const { return path.front(); }
    pair<int, int> to() const { return path.back(); }
};

void initBoard(Board &board) {
    board = Board(BOARD_SIZE, vector<int>(BOARD_SIZE, EMPTY));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if ((i + j) % 2 == 1)
                board[i][j] = P2_MAN;

    for (int i = 5; i < 8; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if ((i + j) % 2 == 1)
                board[i][j] = P1_MAN;
}

void printBoard(const Board &board) {
    cout << "   ";
    for (int j = 0; j < BOARD_SIZE; ++j)
        cout << j << " ";
    cout << "\n";

    for (int i = 0; i < BOARD_SIZE; ++i) {
        cout << i << "  ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            char c = '.';
            switch (board[i][j]) {
                case P1_MAN: c = 'x'; break;
                case P1_KING: c = 'X'; break;
                case P2_MAN: c = 'o'; break;
                case P2_KING: c = 'O'; break;
            }
            cout << c << " ";
        }
        cout << "\n";
    }
    cout << endl;
}

bool isInside(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

void getMultiJumpMoves(Board board, int r, int c, Move move, vector<Move> &result, int player) {
    int piece = board[r][c];
    bool isKing = abs(piece) == 2;
    bool foundJump = false;

    vector<pair<int, int>> directions = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

    for (auto [dr, dc] : directions) {
        if (!isKing && ((player > 0 && dr > 0) || (player < 0 && dr < 0))) continue;

        int midR = r + dr, midC = c + dc;
        int destR = r + 2 * dr, destC = c + 2 * dc;

        if (isInside(destR, destC) && board[midR][midC] * player < 0 && board[destR][destC] == EMPTY) {
            Board temp = board;
            temp[r][c] = EMPTY;
            temp[midR][midC] = EMPTY;
            temp[destR][destC] = piece;

            Move nextMove = move;
            nextMove.addStep({destR, destC});
            nextMove.isCapture = true;

            getMultiJumpMoves(temp, destR, destC, nextMove, result, player);
            foundJump = true;
        }
    }

    if (!foundJump && move.isCapture) {
        result.push_back(move);
    }
}

vector<Move> getLegalMoves(const Board &board, int player) {
    vector<Move> allMoves;
    vector<Move> captureMoves;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            int piece = board[r][c];
            if (piece == EMPTY || (player > 0 && piece < 0) || (player < 0 && piece > 0))
                continue;

            bool isKing = abs(piece) == 2;
            vector<pair<int, int>> directions = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

            for (auto [dr, dc] : directions) {
                if (!isKing && ((player > 0 && dr > 0) || (player < 0 && dr < 0))) continue;

                int nr = r + dr, nc = c + dc;

                if (isInside(nr, nc) && board[nr][nc] == EMPTY) {
                    Move m({r, c});
                    m.addStep({nr, nc});
                    allMoves.push_back(m);
                }

                int mr = r + dr, mc = c + dc;
                int er = r + 2 * dr, ec = c + 2 * dc;

                if (isInside(er, ec) && board[mr][mc] * player < 0 && board[er][ec] == EMPTY) {
                    Board temp = board;
                    temp[r][c] = EMPTY;
                    temp[mr][mc] = EMPTY;
                    temp[er][ec] = piece;

                    Move m({r, c});
                    m.addStep({er, ec});
                    m.isCapture = true;

                    getMultiJumpMoves(temp, er, ec, m, captureMoves, player);
                }
            }
        }
    }

    return !captureMoves.empty() ? captureMoves : allMoves;
}

void applyMove(Board &board, const Move &move, int player) {
    auto [fromR, fromC] = move.from();
    auto [toR, toC] = move.to();

    int piece = board[fromR][fromC];
    board[fromR][fromC] = EMPTY;

    for (size_t i = 1; i < move.path.size(); ++i) {
        int r1 = move.path[i - 1].first;
        int c1 = move.path[i - 1].second;
        int r2 = move.path[i].first;
        int c2 = move.path[i].second;

        if (abs(r2 - r1) == 2 && abs(c2 - c1) == 2) {
            int capR = (r1 + r2) / 2;
            int capC = (c1 + c2) / 2;
            board[capR][capC] = EMPTY;
        }
    }

    board[toR][toC] = piece;

    // King promotion
    if (player == P1_MAN && toR == 0 && piece == P1_MAN) board[toR][toC] = P1_KING;
    if (player == P2_MAN && toR == 7 && piece == P2_MAN) board[toR][toC] = P2_KING;
}

int main() {
    Board board;
    initBoard(board);

    int currentPlayer = P1_MAN;

    while (true) {
        printBoard(board);

        vector<Move> moves = getLegalMoves(board, currentPlayer);

        if (moves.empty()) {
            cout << (currentPlayer == P1_MAN ? "Player 1" : "Player 2") << " has no legal moves. Game over.\n";
            break;
        }

        cout << (currentPlayer == P1_MAN ? "Player 1" : "Player 2") << "'s turn\n";
        for (size_t i = 0; i < moves.size(); ++i) {
            cout << i << ": ";
            for (auto [r, c] : moves[i].path) cout << "(" << r << "," << c << ") ";
            if (moves[i].isCapture) cout << "[capture]";
            cout << "\n";
        }

        int choice;
        cout << "Select move index: ";
        cin >> choice;

        if (choice < 0 || choice >= moves.size()) {
            cout << "Invalid move index.\n";
            continue;
        }

        applyMove(board, moves[choice], currentPlayer);

        currentPlayer = -currentPlayer; // Switch turn
    }

    return 0;
}