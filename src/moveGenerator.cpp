#include "moveGenerator.h"
#include <cstdlib>

//return true if the new position is in the board and is empty or of the other color
bool moveGenerator::validTarget(const BoardState& boardState, int from, int to, int delta)
{
	if (to < 0 || to > 63 || boardState.board[to] != Piece::Empty && isCurrentTurn(boardState.board[to], boardState.whiteToMove))
		return false;
	int fromCol = from % 8;
	int fromRow = from / 8;
	int toCol = to % 8;
	int toRow = to / 8;
	if (abs(toCol - fromCol) > delta || abs(toRow - fromRow) > delta)
		return false;
	return true;
}

//return all the possible move for the king
std::vector<Move> moveGenerator::getKingMoves(const BoardState& boardState, int square, bool castle)
{
	std::vector<Move> moves;
	std::vector<int> kingMoves = { -9, -8, -7, -1, 1, 7, 8, 9 };
	//for every square around the king if the target is valid -> add the move to the moves
	for (int move : kingMoves)
	{
		int target = square + move;
		if (validTarget(boardState, square, target, 1))
			moves.push_back({ square, square + move });
	}
	if (castle)
	{
		//if can castle (kingside), the squares between the rook and the king are empty, the king and the squares were the king "walks" aren't in check -> add the move to the moves
		if (boardState.canCastle[boardState.whiteToMove ? 0 : 2])
		{
			if (boardState.board[5 + (boardState.whiteToMove ? 0 : 56)] == Piece::Empty && boardState.board[6 + (boardState.whiteToMove ? 0 : 56)] == Piece::Empty
				&& !isSquareAttacked(boardState, square) && !isSquareAttacked(boardState, square + 1) && !isSquareAttacked(boardState, square + 2))
				moves.push_back({ square, 6 + (boardState.whiteToMove ? 0 : 56), Piece::Empty, MoveFlag::Castle });
		}
		//if can castle (queenside), the squares between the rook and the king are empty, the king and the squares were the king "walks" aren't in check  -> add the move to the moves
		if (boardState.canCastle[boardState.whiteToMove ? 1 : 3])
		{
			if (boardState.board[1 + (boardState.whiteToMove ? 0 : 56)] == Piece::Empty && boardState.board[2 + (boardState.whiteToMove ? 0 : 56)] == Piece::Empty && boardState.board[3 + (boardState.whiteToMove ? 0 : 56)] == Piece::Empty
				&& !isSquareAttacked(boardState, square) && !isSquareAttacked(boardState, square - 1) && !isSquareAttacked(boardState, square - 2))
				moves.push_back({ square, 2 + (boardState.whiteToMove ? 0 : 56), Piece::Empty, MoveFlag::Castle });
		}
	}
	return moves;
}

//return all the possible move for the queen
std::vector<Move> moveGenerator::getQueenMoves(const BoardState& boardState, int square)
{
	std::vector<Move> moves;
	//add the two vector of moves (rook and bishop) to one and return it
	std::vector<Move> rMoves = getRookMoves(boardState, square);
	std::vector<Move> bMoves = getBishopMoves(boardState, square);
	moves.insert(moves.end(), rMoves.begin(), rMoves.end());
	moves.insert(moves.end(), bMoves.begin(), bMoves.end());
	return moves;
}

//return all the possible move for the rook
std::vector<Move> moveGenerator::getRookMoves(const BoardState& boardState, int square)
{
	std::vector<Move> moves;
	std::vector<int> rookDirections = { -1, +1, -8, +8 };
	//for every direction (N, S, W, E)
	for (int dir : rookDirections)
	{
		int actualPos = square;
		int target;		
		while (true)
		{
			target = actualPos + dir;
			//if the target is on a "friend" target or is out of the board -> break
			if (!validTarget(boardState, actualPos, target, 1))
				break;
			moves.push_back({ square, target });
			//if the target is on a "enemy" target -> break
			if (boardState.board[target] != Piece::Empty)
				break;
			actualPos = target;
		}
	}
	return moves;
}

//return all the possible move for the bishop
//getBishopMoves() is the same of getRookMoves() but with different directions (NE, SE, SW, NW)
std::vector<Move> moveGenerator::getBishopMoves(const BoardState& boardState, int square)
{
	std::vector<Move> moves;
	std::vector<int> bishopDirections = { +7, +9, -7, -9 };
	for (int dir : bishopDirections)
	{
		int actualPos = square;
		int target;
		while (true)
		{
			target = actualPos + dir;
			if (!validTarget(boardState, actualPos, target, 1))
				break;
			moves.push_back({ square, target });
			if (boardState.board[target] != Piece::Empty)
				break;
			actualPos = target;
		}
	}
	return moves;
}

//return all the possible move for the knight
std::vector<Move> moveGenerator::getKnightMoves(const BoardState& boardState, int square)
{
	std::vector<Move> moves;
	std::vector<int> knightMoves = {17, 10, -6, -15, -17, -10, 6, 15};
	//for every move of the knight if the target is valid -> add the move to the moves
	for (int move : knightMoves)
	{
		int target = square + move;
		if (validTarget(boardState, square, target, 2))
			moves.push_back({ square, target });
	}
	return moves;
}

//return all the possible move for the pawn
std::vector<Move> moveGenerator::getPawnMoves(const BoardState& boardState, int square)
{
	std::vector<Move> moves;
	int dir = boardState.whiteToMove ? 1 : -1;
	int oneStep = square + 8 * dir;
	//if the front square is empty
	if (boardState.board[oneStep] == Piece::Empty)
	{
		//if the target is on the last row
		if (oneStep / 8 == 7 || oneStep / 8 == 0)
		{
			std::vector<Piece> pieces;
			if (boardState.whiteToMove)
				pieces = { Piece::wQ, Piece::wR, Piece::wB, Piece::wN };
			else
				pieces = { Piece::bQ, Piece::bR, Piece::bB, Piece::bN };
			//generate and add to the moves 4 moves, each with one option of the type of the promoted piece
			for (Piece promo : pieces)
				moves.push_back({ square, oneStep, promo, MoveFlag::Promotion });
		}
		//if it isn't on the last row
		else
		{
			moves.push_back({ square, oneStep });
			//if the pawn was on the 2th or on the 7th -> if the square in front of the new square is empty -> add the move to the moves
			if (square / 8 == (boardState.whiteToMove ? 1 : 6))
			{
				int twoStep = square + 16 * dir;
				if (boardState.board[twoStep] == Piece::Empty)
					moves.push_back({ square, twoStep });
			}
		}
	}
	//generate the target of an hypotetical capture
	for (int dgMove : {7, 9})
	{
		int target = square + dgMove * dir;

		//if the target is valid

		if (validTarget(boardState, square, target, 1))
		{
			//if the target isn't empty
			if (boardState.board[target] != Piece::Empty)
				//if the target is on the last row
				if (target / 8 == 7 || target / 8 == 0)
				{
					std::vector<Piece> pieces;
					if (boardState.whiteToMove)
						pieces = { Piece::wQ, Piece::wR, Piece::wB, Piece::wN };
					else
						pieces = { Piece::bQ, Piece::bR, Piece::bB, Piece::bN };
					//generate and add to the moves 4 moves, each with one option of the type of the promoted piece
					for (Piece promo : pieces)
						moves.push_back({ square, target, promo, MoveFlag::Promotion });
				}
				else
					moves.push_back({ square, target });
			//if the target is empty and the target is the enpassant target -> add the move to the moves
			else if (boardState.passantTarget == target)
				moves.push_back({ square, target, Piece::Empty, MoveFlag::EnPassant});
		}
	}
	return moves;
}

//find the king in the current position if there isn't return -1
int moveGenerator::findKing(const BoardState& boardState, bool white)
{
	Piece king = white ? Piece::wK : Piece::bK;
	for (int i = 0; i < 64; i++)
	{
		if (boardState.board[i] == king)
			return i;
	}
	return -1;
}

//generate all the of the enemy moves (pseudoLegal), if one move's target is that square -> return true else false;
bool moveGenerator::isSquareAttacked(const BoardState& boardState, int square)
{
	//generate the two positions were a pawn can attack the square -> if in that position there is a pawn -> return true
	for (int dir : {7, 9})
	{
		if (!boardState.whiteToMove)
			dir = -dir;
		int target = dir + square;
		if (validTarget(boardState, square, target, 1))
			if (boardState.board[target] == Piece::bP || boardState.board[target] == Piece::wP)
				return true;
	}
	//generate positions for knight, bishop, rook, king and if in that position there is one of that (for bishop and rook i check if there is a queen too) -> return true
	std::vector<Move> knightMoves = getKnightMoves(boardState, square);
	std::vector<Move> bishopMoves = getBishopMoves(boardState, square);
	std::vector<Move> rookMoves = getRookMoves(boardState, square);
	std::vector<Move> kingMoves = getKingMoves(boardState, square, false);
	for (Move move : knightMoves)
	{
		if (boardState.board[move.to] == Piece::wN || boardState.board[move.to] == Piece::bN)
			return true;
	}
	for (Move move : bishopMoves)
	{
		if (boardState.board[move.to] == Piece::wB || boardState.board[move.to] == Piece::bB || boardState.board[move.to] == Piece::wQ || boardState.board[move.to] == Piece::bQ)
			return true;
	}
	for (Move move : rookMoves)
	{
		if (boardState.board[move.to] == Piece::wR || boardState.board[move.to] == Piece::bR || boardState.board[move.to] == Piece::wQ || boardState.board[move.to] == Piece::bQ)
			return true;
	}
	for (Move move : kingMoves)
	{
		if (boardState.board[move.to] == Piece::wK || boardState.board[move.to] == Piece::bK)
			return true;
	}
	return false;
}

BoardState moveGenerator::applyMove(BoardState& boardState, const Move& move)
{
	bool enPassantChanged = false;

	switch (move.moveFlag)
	{
	//if the move is normal -> the start square became empty and the end square became of the type of the start square
	case MoveFlag::None:
		boardState.board[move.to] = boardState.board[move.from];
		boardState.board[move.from] = Piece::Empty;
		//if a pawn was pushed of two steps -> enpassant = true on the square behind the new one
		if ((boardState.board[move.to] == Piece::wP || boardState.board[move.to] == Piece::bP) && abs(move.to - move.from) == 16)
		{
			boardState.passantTarget = (move.to + move.from) / 2;
			enPassantChanged = true;
		}
		else 
		{
			//if a king moves he can't castle anymore, if a rook moves, the king can't castle in that side
			if (boardState.board[move.to] == Piece::wK || boardState.board[move.to] == Piece::wR && move.from % 8 == 7)
				boardState.canCastle[0] = false;
			else if (boardState.board[move.to] == Piece::bK || boardState.board[move.to] == Piece::bR && move.from % 8 == 7)
				boardState.canCastle[1] = false;
			if (boardState.board[move.to] == Piece::wK || boardState.board[move.to] == Piece::wR && move.from % 8 == 0)
				boardState.canCastle[2] = false;
			else if (boardState.board[move.to] == Piece::bK || boardState.board[move.to] == Piece::bR && move.from % 8 == 0)
				boardState.canCastle[3] = false;
		}
		break;	
	//if the move is a promotion -> the start square became empty and the end square became of the type selected (between: queen, rook, bishop, knight)
	case MoveFlag::Promotion:
		boardState.board[move.to] = move.promotionPiece;
		boardState.board[move.from] = Piece::Empty;
		break;
	//if the move is enpassant -> the start square became empty, the end square became of the type of the start square and the square behind the new square became empty
	case MoveFlag::EnPassant:
		boardState.board[move.to] = boardState.board[move.from];
		boardState.board[move.from] = Piece::Empty;
		boardState.board[move.to - 8 * (boardState.whiteToMove ? 1 : -1)] = Piece::Empty;
		break;
	//if the move is castle the square where the king and rook were became empty, the new square of the king became the square of "move.to" and the square of the rook is the near one
	//(left for the kingside, right for the queenside)
	case MoveFlag::Castle:
		boardState.board[move.to] = boardState.board[move.from];
		boardState.board[move.from] = Piece::Empty;
		if (move.to % 8 == 6)
		{
			boardState.board[move.to - 1] = boardState.whiteToMove ? Piece::wR : Piece::bR;
			boardState.board[move.to + 1] = Piece::Empty;
		}
		else
		{
			boardState.board[move.to + 1] = boardState.whiteToMove ? Piece::wR : Piece::bR;
			boardState.board[move.to -2 ] = Piece::Empty;
		}
		//the player that castled can't castle anymore
		boardState.canCastle[boardState.whiteToMove ? 0 : 2] = false;
		boardState.canCastle[boardState.whiteToMove ? 1 : 3] = false;
		break;
	}
	//if the new move wasn't a pawn pushed of two squares -> on the next turn nobody can do enpassant
	if (!enPassantChanged)
		boardState.passantTarget = -1;
	//change the player to move
	boardState.whiteToMove = !boardState.whiteToMove;
	return boardState;
}

//returns the moves of the piece on the square
std::vector<Move> moveGenerator::getPseudoLegalMoves(const BoardState& boardState, int square)
{
	switch (boardState.board[square])
	{
	case Piece::wK: case Piece::bK: return getKingMoves(boardState, square);
	case Piece::wQ: case Piece::bQ: return getQueenMoves(boardState, square);
	case Piece::wR: case Piece::bR: return getRookMoves(boardState, square);
	case Piece::wB: case Piece::bB: return getBishopMoves(boardState, square);
	case Piece::wN: case Piece::bN: return getKnightMoves(boardState, square);
	case Piece::wP: case Piece::bP: return getPawnMoves(boardState, square);
	default: return {};
	}
}

//return only legal move for a piece on the board
std::vector<Move> moveGenerator::getLegalMoves(const BoardState& boardState, int square)
{
	//find all the pseudoLegal moves
	std::vector<Move> pseudoLegalMoves = getPseudoLegalMoves(boardState, square);
	std::vector<Move> legalMoves;
	for (Move move : pseudoLegalMoves)
	{
		BoardState newBoardState = boardState; //temporary until I create undo() function
		applyMove(newBoardState, move);
		//after applying the pseudoLegal move -> if the king isn't attacked -> add move to legal move
		if (!isSquareAttacked({ newBoardState.board, newBoardState.canCastle, newBoardState.passantTarget, !newBoardState.whiteToMove }, findKing(newBoardState, boardState.whiteToMove)))
			legalMoves.push_back(move);
	}
	return legalMoves;
}

//return all the move for the the player
std::vector<Move> moveGenerator::getAllMoves(const BoardState& boardState)
{
	std::vector<Move> moves;
	for (int i = 0; i < 64; i++)
	{
		Piece piece = boardState.board[i];
		if (piece == Piece::Empty)
			continue;
		if (isCurrentTurn(boardState.board[i], boardState.whiteToMove))
		{
			std::vector<Move> pieceMoves = getPseudoLegalMoves(boardState, i);
			moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
		}
	}
	return moves;
}

//return all the legal moves for a player
//getAllLegalMoves() is the same of getAllMoves() but instead of using getPseudoLegalMove() it use getLegalMove()
std::vector<Move> moveGenerator::getAllLegalMoves(const BoardState& boardState)
{
	std::vector<Move> moves;
	for (int i = 0; i < 64; i++)
	{
		Piece piece = boardState.board[i];
		if (piece == Piece::Empty)
			continue;
		if (isCurrentTurn(boardState.board[i], boardState.whiteToMove))
		{
			std::vector<Move> pieceMoves = getLegalMoves(boardState, i);
			moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
		}
	}
	return moves;
}

Colors moveGenerator::getGameState(BoardState& boardState)
{
	std::vector<Move> legalMoves = getAllLegalMoves(boardState);
	if (legalMoves.empty())
	{
		if (isSquareAttacked(boardState, findKing(boardState, boardState.whiteToMove)))
			return boardState.whiteToMove ? Colors::Black : Colors::White;
		return Colors::Draw;
	}
	else
		return Colors::Continue;
}
