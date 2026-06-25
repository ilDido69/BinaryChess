#include "moveGen.h"
#include "lookUpTables.h"

void MoveGen::generatePawnMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    //get all pawns of the active side
    uint64_t pawns = boardState.bb[side][PAWN];

    while (pawns)
    {
        int from = std::countr_zero(pawns);
        pawns &= pawns - 1;

        //get possible targets, filtering out our own pieces
        uint64_t attacks = LookupTables::pawnAttacks[side][from] & boardState.byColor[enemy];

        //attacks
        while (attacks)
        {
            int to = std::countr_zero(attacks);
            attacks &= attacks - 1;

            if (to / 8 == 7 || to / 8 == 0)
                for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT})
                    out.push(encodeMove(from, to, PAWN, PROMO_CAP, piece));
            else
                out.push(encodeMove(from, to, PAWN, CAPTURE));
        }

        //en passant
        if (boardState.enPassantSq != -1)
            if (LookupTables::pawnAttacks[side][from] & (1ULL << boardState.enPassantSq))
                out.push(encodeMove(from, boardState.enPassantSq, PAWN, EN_PASSANT));

        //one step
        int to = from + (side == WHITE ? 8 : -8);
        if (!(boardState.occupied & (1ULL << to)))
        {
            if (to / 8 == 7 || to / 8 == 0)
                for (Piece piece : {QUEEN, ROOK, BISHOP, KNIGHT})
                    out.push(encodeMove(from, to, PAWN, PROMO, piece));
            else
                out.push(encodeMove(from, to, PAWN));

            //two steps
            if (side == WHITE && from / 8 == 1 || side == BLACK && from / 8 == 6)
            {
                to = from + (side == WHITE ? 16 : -16);
                if (!(boardState.occupied & (1ULL << to)))
                    out.push(encodeMove(from, to, PAWN));
            }
        }
    }
}

void MoveGen::generateKnightMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    //get all knights of the active side
    uint64_t knights = boardState.bb[side][KNIGHT];

    while (knights) 
    {
        int from = std::countr_zero(knights);
        knights &= knights - 1;

        //get possible targets, filtering out our own pieces
        uint64_t attacks = LookupTables::knightAttacks[from] & ~boardState.byColor[side];

        while (attacks) 
        {
            int to = std::countr_zero(attacks);
            attacks &= attacks - 1;

            //determine if it's a capture
            MoveFlag flag = (boardState.byColor[enemy] & (1ULL << to)) ? CAPTURE : QUIET;
            out.push(encodeMove(from, to, KNIGHT, flag));
        }
    }
}

void MoveGen::generateBishopMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    std::array<int, 4> dirs = { -9, -7, 7, 9 };
    //get all bishops of the active side
    uint64_t bishops = boardState.bb[side][BISHOP];

    while (bishops)
    {
        int from = std::countr_zero(bishops);
        bishops &= bishops - 1;

        //for each direction
        for (int i = 0; i < 4; i++)
        {
            int dir = dirs[i];
            uint64_t validTargets = LookupTables::bishopRays[i][from];
            int to = from + dir;

            //while 0<=target<64 and the target is on the targets of the lookuptables 
            while (validTargets & (1ULL << to)) //
            {
                //if the target is occupied
                if (boardState.occupied & (1ULL << to))
                {
                    if (boardState.byColor[side] & (1ULL << to))
                        break;
                    out.push(encodeMove(from, to, BISHOP, CAPTURE));
                    break;
                }

                out.push(encodeMove(from, to, BISHOP));
                to += dir;
            }
        }
    }
}

void MoveGen::generateRookMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;


    std::array<int, 4> dirs = { -8, -1, 1, 8 };
    //get all rooks of the active side
    uint64_t rooks = boardState.bb[side][ROOK];

    while (rooks)
    {
        int from = std::countr_zero(rooks);
        rooks &= rooks - 1; 

        //for each direction
        for (int i = 0; i < 4; i++)
        {
            int dir = dirs[i];
            uint64_t validTargets = LookupTables::rookRays[i][from];
            int to = from + dir;

            //while 0<=target<64 and the target is on the targets of the lookuptables 
            while (validTargets & (1ULL << to)) //
            {
                //if the target is occupied
                if (boardState.occupied & (1ULL << to))
                {
                    if (boardState.byColor[side] & (1ULL << to))
                        break;
                    out.push(encodeMove(from, to, ROOK, CAPTURE));
                    break;
                }

                out.push(encodeMove(from, to, ROOK));
                to += dir;
            }
        }
    }
}

void MoveGen::generateQueenMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    std::array<int, 4> bDirs = { -9, -7, 7, 9 };
    std::array<int, 4> rDirs = { -8, -1, 1, 8 };

    //get all queens of the active side
    uint64_t queens = boardState.bb[side][QUEEN];

    while (queens)
    {
        int from = std::countr_zero(queens);
        queens &= queens - 1;

        //for bishop directions
        for (int i = 0; i < 4; i++)
        {
            int dir = bDirs[i];
            uint64_t validTargets = LookupTables::bishopRays[i][from];
            int to = from + dir;

            //while 0<=target<64 and the target is on the targets of the lookuptables 
            while ( validTargets & (1ULL << to)) //
            {
                //if the target is occupied
                if (boardState.occupied & (1ULL << to))
                {
                    if (boardState.byColor[side] & (1ULL << to))
                        break;
                    out.push(encodeMove(from, to, QUEEN, CAPTURE));
                    break;
                }

                out.push(encodeMove(from, to, QUEEN));
                to += dir;
            }
        }
        //for rook directions
        for (int i = 0; i < 4; i++)
        {
            int dir = rDirs[i];
            uint64_t validTargets = LookupTables::rookRays[i][from];
            int to = from + dir;

            //while 0<=target<64 and the target is on the targets of the lookuptables 
            while (validTargets & (1ULL << to)) //
            {
                if (boardState.occupied & (1ULL << to))
                {
                    //if the target is occupied
                    if (boardState.byColor[side] & (1ULL << to))
                        break;
                    out.push(encodeMove(from, to, QUEEN, CAPTURE));
                    break;
                }

                out.push(encodeMove(from, to, QUEEN));
                to += dir;
            }
        }
    }
}

void MoveGen::generateKingMoves(const BoardState& boardState, MoveList& out)
{
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    //get the king of the active side
    uint64_t king = boardState.bb[side][KING];

    int from = std::countr_zero(king);
    king &= king - 1;

    //get possible targets, filtering out our own pieces
    uint64_t attacks = LookupTables::kingAttacks[from] & ~boardState.byColor[side];

    while (attacks) 
    {
        int to = std::countr_zero(attacks);
        attacks &= attacks - 1;

        //determine if it's a capture
        MoveFlag flag = (boardState.byColor[enemy] & (1ULL << to)) ? CAPTURE : QUIET;
        out.push(encodeMove(from, to, KING, flag));
    }

    //castle
    if (side == WHITE)
    {
        //kingside
        if (boardState.castlingRights & (1ULL ))
            if (!(boardState.occupied & (1ULL << 5 | 1ULL << 6)) && (boardState.bb[side][ROOK] & (1ULL << 7)))
                if (!isAttacked(boardState, from, enemy) && !isAttacked(boardState, from + 1, enemy)) //don't controls from + 2 because after making the move, the king is on from + 2 and isCheck() controls that square
                    out.push(encodeMove(4, 6, KING, CASTLE_K));
        //queenside
        if (boardState.castlingRights & (1ULL << 1))
            if (!(boardState.occupied & (1ULL << 1 | 1ULL << 2 | 1ULL << 3)) && (boardState.bb[side][ROOK] & (1ULL << 0)))
                if (!isAttacked(boardState, from, enemy) && !isAttacked(boardState, from - 1, enemy)) //don't controls ...
                    out.push(encodeMove(4, 2, KING, CASTLE_Q));
    }
    else
    {
        //kingside
        if (boardState.castlingRights & (1ULL << 2))
            if (!(boardState.occupied & (1ULL << 61 | 1ULL << 62)) && (boardState.bb[side][ROOK] & (1ULL << 63)))
                if (!isAttacked(boardState, from, enemy) && !isAttacked(boardState, from + 1, enemy)) //don't controls ...
                    out.push(encodeMove(60, 62, KING, CASTLE_K));
        //queenside
        if (boardState.castlingRights & (1ULL << 3))
            if (!(boardState.occupied & (1ULL << 57 | 1ULL << 58 | 1ULL << 59)) && (boardState.bb[side][ROOK] & (1ULL << 56)))
                if (!isAttacked(boardState, from, enemy) && !isAttacked(boardState, from - 1, enemy)) //don't controls ...
                    out.push(encodeMove(60, 58, KING, CASTLE_Q));
    }
}

//all pseudo-legal moves
void MoveGen::getAllMoves(const BoardState& boardState, MoveList& out)
{
    generateKingMoves(boardState, out);
    generateQueenMoves(boardState, out);
    generateRookMoves(boardState, out);
    generateBishopMoves(boardState, out);
    generateKnightMoves(boardState, out);
    generatePawnMoves(boardState, out);
}

//all legal moves
void MoveGen::getLegalMoves(BoardState& boardState, MoveList& out)
{
    getAllMoves(boardState, out);
    StateInfo saved;

    //for each move => makeMove -> if the the king of the side that moved is in check => the move is illegal -> remove the move from the moveList 
    for (int i = 0; i < out.count; i++)
    {
        makeMove(boardState, out.moves[i], saved);
        bool illegal = isCheck(boardState);
        unmakeMove(boardState, out.moves[i], saved);

        if (illegal)
        {
            out.remove(i--);
        }
    }
}

//update boardState and stateInfo, the move must be verified before
void MoveGen::makeMove(BoardState& boardState, Move move, StateInfo& saved)
{
    int from = getFrom(move);
    int to = getTo(move);
    Piece moved = getMoved(move);
    MoveFlag moveFlag = getFlag(move);
    Piece promoPiece = getPromo(move);
    Color side = boardState.sideToMove;
    Color enemy = ~side;

    //update stateInfo
    saved.rule50 = boardState.rule50;
    saved.enPassantSquare = boardState.enPassantSq;
    saved.castlingRights = boardState.castlingRights;
    saved.capturedPiece = EMPTY;

    //change bitboards
    switch (moveFlag)
    {
    case QUIET:
    {
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][moved], to);
        clearBit(boardState.occupied, from);
        setBit(boardState.occupied, to);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        break;
    }
    case CAPTURE:
    {
        Piece capturedPiece = getPiece(boardState, enemy, to);
        saved.capturedPiece = capturedPiece;
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][moved], to);
        clearBit(boardState.bb[enemy][capturedPiece], to);
        clearBit(boardState.occupied, from);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        clearBit(boardState.byColor[enemy], to);
        break;
    }
    case EN_PASSANT:
    {
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][moved], to);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        clearBit(boardState.occupied, from);
        setBit(boardState.occupied, to);
        int enPassantPawn = (from / 8) * 8 + to % 8;
        clearBit(boardState.bb[enemy][PAWN], enPassantPawn);
        clearBit(boardState.byColor[enemy], enPassantPawn);
        clearBit(boardState.occupied, enPassantPawn);
        break;
    }
    case CASTLE_K:
    {
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][moved], to);
        clearBit(boardState.bb[side][ROOK], from + 3);
        setBit(boardState.bb[side][ROOK], from + 1);
        clearBit(boardState.occupied, from);
        setBit(boardState.occupied, to);
        clearBit(boardState.occupied, from + 3);
        setBit(boardState.occupied, from + 1);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        clearBit(boardState.byColor[side], from + 3);
        setBit(boardState.byColor[side], from + 1);
        break;
    }
    case CASTLE_Q:
    {
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][moved], to);
        clearBit(boardState.bb[side][ROOK], from - 4);
        setBit(boardState.bb[side][ROOK], from - 1);
        clearBit(boardState.occupied, from);
        setBit(boardState.occupied, to);
        clearBit(boardState.occupied, from - 4);
        setBit(boardState.occupied, from - 1);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        clearBit(boardState.byColor[side], from - 4);
        setBit(boardState.byColor[side], from - 1);
        break;
    }
    case PROMO:
    {
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][promoPiece], to);
        clearBit(boardState.occupied, from);
        setBit(boardState.occupied, to);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        break;
    }
    case PROMO_CAP:
    {
        Piece capturedPiece = getPiece(boardState, enemy, to);
        saved.capturedPiece = capturedPiece;
        clearBit(boardState.bb[side][moved], from);
        setBit(boardState.bb[side][promoPiece], to);
        clearBit(boardState.bb[enemy][capturedPiece], to);
        clearBit(boardState.occupied, from);
        clearBit(boardState.byColor[side], from);
        setBit(boardState.byColor[side], to);
        clearBit(boardState.byColor[enemy], to);
        break;
    }
    }

    //add move to rule50 and set to None en passant (rule50 will be reset to zero later, and enPassant will be set to a valid square later)
    boardState.enPassantSq = -1;
    boardState.rule50++;

    //set boardstate variables (no bitboards)
    if (moved == PAWN)
    {
        if (to - from == 16 || from - to == 16)
        {
            boardState.enPassantSq = (to + from) / 2;
        }
        boardState.rule50 = 0;
    }
    else if (moved == KING)
    {
        if (side == WHITE)
            boardState.castlingRights &= ~0x3;
        else
            boardState.castlingRights &= ~(0x3 << 2);
    }
    else if (moved == ROOK)
    {
        if (from == 0)
            boardState.castlingRights &= ~(1 << 1);
        else if (from == 7)
            boardState.castlingRights &= ~(1 << 0);
        else if (from == 56)
            boardState.castlingRights &= ~(1 << 3);
        else if (from == 63)
            boardState.castlingRights &= ~(1 << 2);
    }
    if (moveFlag == CAPTURE || moveFlag == PROMO_CAP)
    {
        if (to == 0)  boardState.castlingRights &= ~(1 << 1);
        else if (to == 7)  boardState.castlingRights &= ~(1 << 0);
        else if (to == 56) boardState.castlingRights &= ~(1 << 3);
        else if (to == 63) boardState.castlingRights &= ~(1 << 2);
        boardState.rule50 = 0;
    }
    
    //change color to move
    boardState.sideToMove = ~boardState.sideToMove;
}

void MoveGen::unmakeMove(BoardState& boardState, Move move, const StateInfo& saved)
{
    int from = getFrom(move);
    int to = getTo(move);
    Piece moved = getMoved(move);
    MoveFlag moveFlag = getFlag(move);
    Piece promoPiece = getPromo(move);
    Color side = ~boardState.sideToMove;
    Color enemy = ~side;

    //change bitboards
    switch (moveFlag)
    {
    case QUIET:
    {
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][moved], to);
        setBit(boardState.occupied, from);
        clearBit(boardState.occupied, to);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        break;
    }
    case CAPTURE:
    {
        Piece capturedPiece = saved.capturedPiece;
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][moved], to);
        setBit(boardState.bb[enemy][capturedPiece], to);
        setBit(boardState.occupied, from);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        setBit(boardState.byColor[enemy], to);
        break;
    }
    case EN_PASSANT:
    {
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][moved], to);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        setBit(boardState.occupied, from);
        clearBit(boardState.occupied, to);
        int enPassantPawn = (from / 8) * 8 + to % 8;
        setBit(boardState.bb[enemy][PAWN], enPassantPawn);
        setBit(boardState.byColor[enemy], enPassantPawn);
        setBit(boardState.occupied, enPassantPawn);
        break;
    }
    case CASTLE_K:
    {
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][moved], to);
        setBit(boardState.bb[side][ROOK], from + 3);
        clearBit(boardState.bb[side][ROOK], from + 1);
        setBit(boardState.occupied, from);
        clearBit(boardState.occupied, to);
        setBit(boardState.occupied, from + 3);
        clearBit(boardState.occupied, from + 1);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        setBit(boardState.byColor[side], from + 3);
        clearBit(boardState.byColor[side], from + 1);
        break;
    }
    case CASTLE_Q:
    {
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][moved], to);
        setBit(boardState.bb[side][ROOK], from - 4);
        clearBit(boardState.bb[side][ROOK], from - 1);
        setBit(boardState.occupied, from);
        clearBit(boardState.occupied, to);
        setBit(boardState.occupied, from - 4);
        clearBit(boardState.occupied, from - 1);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        setBit(boardState.byColor[side], from - 4);
        clearBit(boardState.byColor[side], from - 1);
        break;
    }
    case PROMO:
    {
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][promoPiece], to);
        setBit(boardState.occupied, from);
        clearBit(boardState.occupied, to);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        break;
    }
    case PROMO_CAP:
    {
        Piece capturedPiece = saved.capturedPiece;
        setBit(boardState.bb[side][moved], from);
        clearBit(boardState.bb[side][promoPiece], to);
        setBit(boardState.bb[enemy][capturedPiece], to);
        setBit(boardState.occupied, from);
        setBit(boardState.byColor[side], from);
        clearBit(boardState.byColor[side], to);
        setBit(boardState.byColor[enemy], to);
        break;
    }
    }

    //reset boardstate variables (no bitboards) from savedInfo
    boardState.rule50 = saved.rule50;
    boardState.enPassantSq = saved.enPassantSquare;
    boardState.castlingRights = saved.castlingRights;

    //change color
    boardState.sideToMove = ~boardState.sideToMove;
}


bool MoveGen::isAttacked(const BoardState& boardState, int sq, Color attacker)
{
    std::array<int, 4> bDirs = { -9, -7, 7, 9 };
    std::array<int, 4> rDirs = { -8, -1, 1, 8 };

    //by knight
    if (LookupTables::knightAttacks[sq] & boardState.bb[attacker][KNIGHT])
        return true;

    //by king
    if (LookupTables::kingAttacks[sq] & boardState.bb[attacker][KING])
        return true;

    //by rook or queen
    for (int i = 0; i < 4; i++)
    {
        int dir = rDirs[i];
        uint64_t rookQueenPos = LookupTables::rookRays[i][sq];

        int to = sq + dir;

        while (rookQueenPos & (1ULL << to)) //
        {
            if (boardState.occupied & (1ULL << to))
            {
                if ((boardState.bb[attacker][ROOK] | boardState.bb[attacker][QUEEN]) & (1ULL << to))
                    return true;
                break;
            }
            to += dir;
        }
    }

    //by bishop or queen
    for (int i = 0; i < 4; i++)
    {
        int dir = bDirs[i];

        uint64_t bishopQueenPos = LookupTables::bishopRays[i][sq];

        int to = sq + dir;

        while (bishopQueenPos & (1ULL << to)) //
        {
            if (boardState.occupied & (1ULL << to))
            {
                if ((boardState.bb[attacker][BISHOP] | boardState.bb[attacker][QUEEN]) & (1ULL << to))
                    return true;
                break;
            }
            to += dir;
        }
    }

    //by pawn
    if (LookupTables::pawnAttacks[~attacker][sq] & boardState.bb[attacker][PAWN])
        return true;


    return false;
}

//return the square were there is the king. the color of the king is an 
int MoveGen::findKing(const BoardState& boardState, Color color)
{
    return std::countr_zero(boardState.bb[color][KING]);
}

//return true if in that position the moving side can attack the king of the other side
bool MoveGen::isCheck(const BoardState& boardState)
{
    return isAttacked(boardState, findKing(boardState, ~boardState.sideToMove), boardState.sideToMove);
}

bool MoveGen::onCheck(const BoardState& boardState)
{
    return isAttacked(boardState, findKing(boardState, boardState.sideToMove),~boardState.sideToMove);
}

//reset the boardstate to the starting position
void MoveGen::resetBoardState(BoardState& boardState)
{
    boardState.occupied = 0xFFFF00000000FFFF;
    boardState.byColor[WHITE] = 0x000000000000FFFF;
    boardState.byColor[BLACK] = 0xFFFF000000000000;
    boardState.bb[WHITE][PAWN] = 0x000000000000FF00;
    boardState.bb[BLACK][PAWN] = 0x00FF000000000000;
    boardState.bb[WHITE][KNIGHT] = 0x0000000000000042;
    boardState.bb[BLACK][KNIGHT] = 0x4200000000000000;
    boardState.bb[WHITE][BISHOP] = 0x0000000000000024;
    boardState.bb[BLACK][BISHOP] = 0x2400000000000000;
    boardState.bb[WHITE][ROOK] = 0x0000000000000081;
    boardState.bb[BLACK][ROOK] = 0x8100000000000000;
    boardState.bb[WHITE][QUEEN] = 0x0000000000000008;
    boardState.bb[BLACK][QUEEN] = 0x0800000000000000;
    boardState.bb[WHITE][KING] = 0x0000000000000010;
    boardState.bb[BLACK][KING] = 0x1000000000000000;
    boardState.castlingRights = 0xF;
    boardState.enPassantSq = -1;
    boardState.rule50 = 0;
    boardState.sideToMove = WHITE;
}

//reset the boardState to the fen position, fen must be validated before
void MoveGen::resetBoardState(BoardState& boardState, std::string fen)
{
    std::string position;
    char color;
    std::string castling;
    std::string enPassant;
    std::string rule50;
    std::string nMoves; // for now not used
    int i = 0;

    boardState = BoardState{};

    while (true)
    {
        if (fen[i] == ' ')
            break;
        position += fen[i];
        i++;
    }

    color = fen[++i];

    i += 2;
    while (true)
    {
        if (fen[i] == ' ')
        {
            i++;
            break;
        }
        castling += fen[i];
        i++;
    }
    
    while (true)
    {
        if (fen[i] == ' ')
        {
            i++;
            break;
        }
        enPassant += fen[i];
        i++;
    }

    while (true)
    {
        if (fen[i] == ' ')
        {
            i++;
            break;
        }
        rule50 += fen[i];
        i++;
    }

    while (true)
    {
        if (fen[i] == ' ')
        {
            i++;
            break;
        }
        nMoves += fen[i];
        i++;
    }
    
    i = 56;
    for (char c : position)
    {
        if (c == '/')
        {
            i -= 16;
            continue;
        }
            

        bool flag = false;
        for (char n : "12345678")
            if (c == n)
            {
                flag = true;
                break;
            }
        if (flag)
            i += c - '0';
        else
        {
            boardState.occupied |= (1ULL << i);
            
            if (c <= 'Z')
                boardState.byColor[WHITE] |= (1ULL << i);
            else
                boardState.byColor[BLACK] |= (1ULL << i);

            switch (c)
            {
            case 'K':
                boardState.bb[WHITE][KING] |= (1ULL << i);
                break;
            case 'k':
                boardState.bb[BLACK][KING] |= (1ULL << i);
                break;
            case 'Q':
                boardState.bb[WHITE][QUEEN] |= (1ULL << i);
                break;
            case 'q':
                boardState.bb[BLACK][QUEEN] |= (1ULL << i);
                break;
            case 'R':
                boardState.bb[WHITE][ROOK] |= (1ULL << i);
                break;
            case 'r':
                boardState.bb[BLACK][ROOK] |= (1ULL << i);
                break;
            case 'B':
                boardState.bb[WHITE][BISHOP] |= (1ULL << i);
                break;
            case 'b':
                boardState.bb[BLACK][BISHOP] |= (1ULL << i);
                break;
            case 'N':
                boardState.bb[WHITE][KNIGHT] |= (1ULL << i);
                break;
            case 'n':
                boardState.bb[BLACK][KNIGHT] |= (1ULL << i);
                break;
            case 'P':
                boardState.bb[WHITE][PAWN] |= (1ULL << i);
                break;
            case 'p':
                boardState.bb[BLACK][PAWN] |= (1ULL << i);
                break;
            }
            i++;
        }
    }

    boardState.sideToMove = color == 'w' ? WHITE : BLACK;

    boardState.castlingRights = 0;
    if (castling != "-")
        for (char c : castling)
        {
            switch (c)
            {
            case 'K':
                boardState.castlingRights |= (1 << 0);
                break;
            case 'k':
                boardState.castlingRights |= (1 << 2);
                break;
            case 'Q':
                boardState.castlingRights |= (1 << 1);
                break;
             case 'q':
                boardState.castlingRights |= (1 << 3);
                break;
            }
        }

    if (enPassant == "-")
        boardState.enPassantSq = -1;
    else
    {
        int c = enPassant[0] - 'a';
        int r = enPassant[1] - '1';
        boardState.enPassantSq = r * 8 + c;
    }

    boardState.rule50 = std::stoi(rule50);
}

//return 0 = white did checkmate, 1 black did checkmate, 2 draw, 3 continue
int MoveGen::getGameState(BoardState boardState)
{
    MoveList moves;
    getLegalMoves(boardState, moves);
    if (moves.count == 0)
    {
        if (onCheck(boardState))
            return boardState.sideToMove == WHITE ? 1 : 0;
        return 2;
    }
    return 3;
}

