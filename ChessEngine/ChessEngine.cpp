// ChessEngine.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "ChessEngine.h"


#pragma region CInterface
// This is an example of an exported function.
extern "C" CHESSENGINE_API CChessEngine * CreateChessEngineClass(void)
{
	return new CChessEngine();
}

extern "C" CHESSENGINE_API void DisposeChessEngineClass(CChessEngine * pObject)
{
	if (pObject != NULL)
	{
		delete pObject;
		pObject = NULL;
	}
}
extern "C" CHESSENGINE_API int GetPiecePosition(CChessEngine * pObject, int piece)
{
	if (pObject != NULL)
	{
		return pObject->GetPiecePositionInt((PIECE_ORDER)piece);
	}
	return -1;
}
extern "C" CHESSENGINE_API int CheckMailbox(CChessEngine* pObject, char pos)
{
	if (pObject != NULL)
	{
		return pObject->CheckMailbox(pos);
	}
	return -1;
}
extern "C" CHESSENGINE_API bool IsEnPassantTile(CChessEngine* pObject, char pos)
{
	if (pObject != NULL)
	{
		return pObject->IsEnPassantTile(pos);
	}
	return false;
}
extern "C" CHESSENGINE_API bool IsEmpty(CChessEngine* pObject, char pos)
{
	if (pObject != NULL)
	{
		return pObject->IsEmpty(pos);
	}
	return false;
}
extern "C" CHESSENGINE_API int GetPressure(CChessEngine* pObject, char pos)
{
	if (pObject != NULL)
	{
		return pObject->GetPressure(pos);
	}
	return 0;
}
extern "C" CHESSENGINE_API int GetValue(CChessEngine* pObject, int piece)
{
	if (pObject != NULL)
	{
		return pObject->GetValue((PIECE_ORDER)piece);
	}
	return 0;
}
extern "C" CHESSENGINE_API bool IsInMoveBitBoard(CChessEngine* pObject, char pos, int piece)
{
	if (pObject != NULL)
	{
		return pObject->IsInMoveBitBoard(pos, (PIECE_ORDER)piece);
	}
	return false;
}
extern "C" CHESSENGINE_API bool IsInAttackBitBoard(CChessEngine* pObject, char pos, int piece)
{
	if (pObject != NULL)
	{
		return pObject->IsInAttackBitBoard(pos, (PIECE_ORDER)piece);
	}
	return false;
}
extern "C" CHESSENGINE_API bool IsInPressureBitBoard(CChessEngine* pObject, char pos, int piece)
{
	if (pObject != NULL)
	{
		return pObject->IsInPressureBitBoard(pos, (PIECE_ORDER)piece);
	}
	return false;
}
extern "C" CHESSENGINE_API void DespawnPiece(CChessEngine* pObject, int piece)
{
	if (pObject != NULL)
	{
		pObject->DespawnPiece((PIECE_ORDER)piece);
	}
}
extern "C" CHESSENGINE_API bool SpawnPiece(CChessEngine* pObject, int piece, char spawn)
{
	if (pObject != NULL)
	{
		return pObject->SpawnPiece((PIECE_ORDER)piece, Vector2Int(spawn));
	}
	return false;
}
extern "C" CHESSENGINE_API bool TryMove(CChessEngine* pObject, int piece, char pos)
{
	if (pObject != NULL)
	{
		return pObject->TryMove((PIECE_ORDER)piece, Vector2Int(pos));
	}
	return false;
}
#pragma endregion

#pragma region EngineClass

#pragma region Private

void CChessEngine::SetOccupied(char pos)
{
	OccupancyBitBoard |= (1LL << (pos & POSITION_MASK));
}

void CChessEngine::SetUnoccupied(char pos)
{
	OccupancyBitBoard &= ~(1LL << (pos & POSITION_MASK));
}

bool CChessEngine::IsSpawned(PIECE_ORDER piece)
{
	return (PiecePositions[(int)piece] & SPAWN_MASK);
}

bool CChessEngine::HasMoved(PIECE_ORDER piece)
{
	return (PiecePositions[(int)piece] & MOVED_MASK);
}

bool CChessEngine::AreDifferentColor(PIECE_ORDER pieceOne, PIECE_ORDER pieceTwo)
{
	return (pieceOne > WHITE_KING != pieceTwo > WHITE_KING);
}

bool CChessEngine::CheckOOB(Vector2Int pos)
{
	return (pos.x >= BOARD_DIMENSIONS || pos.x < 0 || pos.y >= BOARD_DIMENSIONS || pos.y < 0);
}

bool CChessEngine::CheckOOB(char pos)
{
	return CheckOOB(Vector2Int(pos & POSITION_MASK));
}
//does not get rid of flags
char CChessEngine::GetPosition(PIECE_ORDER piece)
{
	return PiecePositions[(int)piece];
}

bool CChessEngine::IsPawn(PIECE_ORDER piece)
{
	return piece < WHITE_ROOK_A || (piece >= BLACK_PAWN_A && piece < BLACK_ROOK_A);
}
bool CChessEngine::EmptyAndInBounds(Vector2Int tile)
{
	return !CheckOOB(tile) && IsEmpty(tile.Convert());
}
bool CChessEngine::PawnAttackCheck(Vector2Int pos, PIECE_ORDER piece)
{
	return !CheckOOB(pos) &&
		(!IsEmpty(pos.Convert()) && AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert()))
			||
			(IsEnPassantTile(pos.Convert()) && AreDifferentColor(piece, EnPassantPiece))
			);
}
bool CChessEngine::HasEnemyAndInBounds(Vector2Int pos, PIECE_ORDER attackingPiece)
{
	return !CheckOOB(pos) && !IsEmpty(pos.Convert()) && AreDifferentColor(attackingPiece, (PIECE_ORDER)CheckMailbox(pos.Convert()));
}
void CChessEngine::SetPosition(char position, PIECE_ORDER piece)
{
	if (((position & SPAWN_MASK) | (position & MOVED_MASK)))
	{
		return;
	}
	//flag the previous as 
	Vector2Int xyPos = Vector2Int(position);
	Vector2Int prevPos = Vector2Int((PiecePositions[(int)piece] & POSITION_MASK));
	int diff = xyPos.y - prevPos.y;
	if (diff < 0)
	{
		diff *= -1;
	}
	if (IsPawn(piece) && diff == 2)
	{
		Vector2Int EnPassantPos = prevPos;
		if (piece >= BLACK_PAWN_A)
		{
			EnPassantPos.y--;
		}
		else
		{
			EnPassantPos.y++;
		}
		EnPassantBitBoard = (1LL << EnPassantPos.Convert());
		EnPassantPiece = piece;
	}
	else
	{
		EnPassantPiece = PIECE_ORDER_SIZE;
		EnPassantBitBoard = 0LL;
	}

	PIECE_ORDER CastlePiece = (PIECE_ORDER)PIECE_ORDER_SIZE;
	Vector2Int CastlePos = Vector2Int();
	//castling
	if (!HasMoved(piece))
	{
		if (piece == WHITE_KING)
		{
			if (position == 2)
			{
				CastlePiece = (PIECE_ORDER)CheckMailbox(Vector2Int(0, 0).Convert());
				CastlePos = Vector2Int(3, 0);
			}
			else if (position == 6)
			{
				CastlePiece = (PIECE_ORDER)CheckMailbox(Vector2Int(7, 0).Convert());
				CastlePos = Vector2Int(5, 0);
			}
		}
		else if (piece == BLACK_KING)
		{
			if (position == 58)
			{
				CastlePiece = (PIECE_ORDER)CheckMailbox(Vector2Int(0, 7).Convert());
				CastlePos = Vector2Int(3, 7);
			}
			else if (position == 62)
			{
				CastlePiece = (PIECE_ORDER)CheckMailbox(Vector2Int(7, 7).Convert());
				CastlePos = Vector2Int(5, 7);
			}
		}
	}

	SetUnoccupied((PiecePositions[(int)piece] & POSITION_MASK));
	Mailbox[(PiecePositions[(int)piece] & POSITION_MASK)] = PIECE_ORDER_SIZE;
	//combine passed in position with flags in actual array
	PiecePositions[(int)piece] = (position | (PiecePositions[(int)piece] & SPAWN_MASK) | MOVED_MASK);
	//flag the occupancy bitboard
	SetOccupied(position);
	//update mailbox
	Mailbox[(PiecePositions[(int)piece] & POSITION_MASK)] = piece;


	if (CastlePiece != PIECE_ORDER_SIZE)
	{
		SetUnoccupied((PiecePositions[(int)CastlePiece] & POSITION_MASK));
		Mailbox[(PiecePositions[(int)CastlePiece] & POSITION_MASK)] = PIECE_ORDER_SIZE;
		PiecePositions[(int)CastlePiece] = (CastlePos.Convert() | (PiecePositions[(int)piece] & SPAWN_MASK) | MOVED_MASK);
		SetOccupied(PiecePositions[(int)CastlePiece]);
		Mailbox[(PiecePositions[(int)CastlePiece] & POSITION_MASK)] = CastlePiece;
	}
	//regenerate bit boards
	RegenerateAllPieceBoards();
}
void CChessEngine::RegenerateAllPieceBoards()
{
	for (char i = 0; i < PIECE_TOTAL; i++)
	{
		//generate bit boards
		GenerateAttackAndMoveBoards((PIECE_ORDER)i);
	}

	for (char i = 0; i < BOARD_SIZE; i++)
	{
		PressureBoard[i] = 0;
		for (char j = 0; j < PIECE_TOTAL; j++)
		{
			//check bitboards for pressure
			if (IsInPressureBitBoard(i, (PIECE_ORDER)j))
			{
				PressureBoard[i] = (j >= (int)BLACK_PAWN_A ? PressureBoard[i] - GetValue((PIECE_ORDER)j) : PressureBoard[i] + GetValue((PIECE_ORDER)j));
			}
		}
	}
}
void CChessEngine::GenerateAttackAndMoveBoards(PIECE_ORDER piece)
{
	if (IsSpawned(piece))
	{
		Vector2Int StartPos = Vector2Int(GetPosition(piece));
		Vector2Int pos = StartPos;
		switch (piece)
		{
		case WHITE_PAWN_A:
		case WHITE_PAWN_B:
		case WHITE_PAWN_C:
		case WHITE_PAWN_D:
		case WHITE_PAWN_E:
		case WHITE_PAWN_F:
		case WHITE_PAWN_G:
		case WHITE_PAWN_H:
		{
			pos.y++;
			PiecePressureBitBoards[(int)piece] = 0LL;
			PieceAttackBitBoards[(int)piece] = 0LL;
			PieceMoveBitBoards[(int)piece] = 0LL;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!HasMoved(piece))
			{
				pos.y++;
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
			}


			pos = StartPos;
			pos.x++;
			pos.y++;
			if (PawnAttackCheck(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			if (!CheckOOB(pos))
			{
				//as long as it's a square in the board, mark it for pressure
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos.x -= 2;
			if (PawnAttackCheck(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			if (!CheckOOB(pos))
			{
				//as long as it's a square in the board, mark it for pressure
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			break;
		}
		case BLACK_PAWN_A:
		case BLACK_PAWN_B:
		case BLACK_PAWN_C:
		case BLACK_PAWN_D:
		case BLACK_PAWN_E:
		case BLACK_PAWN_F:
		case BLACK_PAWN_G:
		case BLACK_PAWN_H:
		{
			pos.y--;
			PieceMoveBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;
			PieceAttackBitBoards[(int)piece] = 0LL;

			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());

				if (!HasMoved(piece))
				{
					pos.y--;
					if (EmptyAndInBounds(pos))
					{
						PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
				}
			}

			pos = StartPos;
			pos.x++;
			pos.y--;
			if (PawnAttackCheck(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				//as long as it's a square in the board, mark it for pressure
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos.x -= 2;
			if (PawnAttackCheck(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			if (!CheckOOB(pos))
			{
				//as long as it's a square in the board, mark it for pressure
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			break;
		}
		case WHITE_ROOK_A:
		case WHITE_ROOK_B:
		case BLACK_ROOK_A:
		case BLACK_ROOK_B:
		{
			//check all 4 directions until we hit something that stops us
			PieceAttackBitBoards[(int)piece] = 0LL;
			PieceMoveBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;
			//up
			while (true)
			{
				pos.y++;

				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}

			}
			pos = StartPos;
			//down
			while (true)
			{
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}

			}
			pos = StartPos;
			//right
			while (true)
			{
				pos.x++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}

			}
			pos = StartPos;
			//left
			while (true)
			{
				pos.x--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}

			}
			break;
		}
		case WHITE_KNIGHT_A:
		case WHITE_KNIGHT_B:
		case BLACK_KNIGHT_A:
		case BLACK_KNIGHT_B:
		{
			PieceAttackBitBoards[(int)piece] = 0LL;
			PieceMoveBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;

			pos = StartPos + Vector2Int(2, 1);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			pos = StartPos + Vector2Int(1, 2);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(-2, 1);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(-1, 2);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(2, -1);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(1, -2);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(-2, -1);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			pos = StartPos + Vector2Int(-1, -2);
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			break;
		}
		case WHITE_BISHOP_A:
		case WHITE_BISHOP_B:
		case BLACK_BISHOP_A:
		case BLACK_BISHOP_B:
		{
			//check all 4 directions until we hit something that stops us
			PieceAttackBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;
			PieceMoveBitBoards[(int)piece] = 0LL;
			while (true)
			{
				pos.x++;
				pos.y++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}

			}
			pos = StartPos;
			while (true)
			{
				pos.x--;
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x++;
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x--;
				pos.y++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			break;
		}
		case WHITE_QUEEN:
		case BLACK_QUEEN:
		{
			//check all 4 directions until we hit something that stops us
			PieceAttackBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;
			PieceMoveBitBoards[(int)piece] = 0LL;
			//up
			while (true)
			{
				pos.y++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			//down
			while (true)
			{
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			//right
			while (true)
			{
				pos.x++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			//left
			while (true)
			{
				pos.x--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x++;
				pos.y++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x--;
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x++;
				pos.y--;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			pos = StartPos;
			while (true)
			{
				pos.x--;
				pos.y++;
				if (!CheckOOB(pos))
				{
					PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				if (EmptyAndInBounds(pos))
				{
					PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
				}
				else
				{
					if (HasEnemyAndInBounds(pos, piece))
					{
						PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
					}
					break;
				}
			}
			break;
		}
		case WHITE_KING:
		case BLACK_KING:
		{
			PieceMoveBitBoards[(int)piece] = 0LL;
			PieceAttackBitBoards[(int)piece] = 0LL;
			PiecePressureBitBoards[(int)piece] = 0LL;

			//up
			pos.y++;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//top right
			pos.x++;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//right
			pos.y--;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//bottom right
			pos.y--;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//bottom
			pos.x--;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//bottom left
			pos.x--;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//left
			pos.y++;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//up left
			pos.y++;
			if (EmptyAndInBounds(pos))
			{
				PieceMoveBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			else if (HasEnemyAndInBounds(pos, piece))
			{
				PieceAttackBitBoards[(int)piece] |= (1LL << pos.Convert());
			}
			if (!CheckOOB(pos))
			{
				PiecePressureBitBoards[(int)piece] |= (1LL << pos.Convert());
			}

			//castling
			//king can't have moved
			if (!HasMoved(piece))
			{
				//which king are we?
				int y = piece == WHITE_KING ? 0 : 7;

				//Check left rook
				PIECE_ORDER rook = (PIECE_ORDER)CheckMailbox(Vector2Int(0, y).Convert());
				bool isRook = piece == WHITE_KING ? (rook == WHITE_ROOK_A || rook == WHITE_ROOK_B) :
					(rook == BLACK_ROOK_A || rook == BLACK_ROOK_B);

				if (isRook && !HasMoved(rook))
				{
					if (IsEmpty(Vector2Int(1, y).Convert()) && IsEmpty(Vector2Int(2, y).Convert()) && IsEmpty(Vector2Int(3, y).Convert()))
					{
						//we can castle
						PieceMoveBitBoards[(int)piece] |= (1LL << (Vector2Int(2, y).Convert()));
					}
				}

				//Check right rook
				rook = (PIECE_ORDER)CheckMailbox(Vector2Int(7, y).Convert());
				isRook = piece == WHITE_KING ? (rook == WHITE_ROOK_A || rook == WHITE_ROOK_B) :
					(rook == BLACK_ROOK_A || rook == BLACK_ROOK_B);

				if (isRook && !HasMoved(rook))
				{
					if (IsEmpty(Vector2Int(5, y).Convert()) && IsEmpty(Vector2Int(6, y).Convert()))
					{
						//we can castle
						PieceMoveBitBoards[(int)piece] |= (1LL << (Vector2Int(6, y).Convert()));
					}
				}
			}


			break;
		}

		}
		PiecePressureBitBoards[(int)piece] |= (1LL << (StartPos.Convert()));
	}

}


#pragma endregion


#pragma region Public


// This is the constructor of a class that has been exported.
CChessEngine::CChessEngine()
{
	OccupancyBitBoard = 0;
	EnPassantBitBoard = 0;
	EnPassantPiece = PIECE_ORDER_SIZE;
	for (size_t i = 0; i < PIECE_TOTAL; i++)
	{
		PiecePositions[i] = 0;
		PieceMoveBitBoards[i] = 0;
		PieceAttackBitBoards[i] = 0;
		PiecePressureBitBoards[i] = 0;
	}

	for (size_t i = 0; i < BOARD_SIZE; i++)
	{
		Mailbox[i] = PIECE_ORDER_SIZE;
	}
	return;
}

CChessEngine::~CChessEngine()
{
}

int CChessEngine::GetPiecePositionInt(PIECE_ORDER piece)
{
	if (!IsSpawned(piece))
	{
		return -1;
	}
	return (GetPosition(piece) & POSITION_MASK);
}
int CChessEngine::CheckMailbox(char pos)
{
	return Mailbox[pos];
}

bool CChessEngine::IsEnPassantTile(char pos)
{
	return (EnPassantBitBoard & (1LL << (pos & POSITION_MASK)));
}

bool CChessEngine::IsEmpty(char pos)
{
	return !(OccupancyBitBoard & (1LL << (pos & POSITION_MASK)));
}
int CChessEngine::GetPressure(char pos)
{
	return PressureBoard[pos & POSITION_MASK];
}
int CChessEngine::GetValue(PIECE_ORDER piece)
{
	switch (piece)
	{
	case WHITE_PAWN_A:
	case WHITE_PAWN_B:
	case WHITE_PAWN_C:
	case WHITE_PAWN_D:
	case WHITE_PAWN_E:
	case WHITE_PAWN_F:
	case WHITE_PAWN_G:
	case WHITE_PAWN_H:
	case BLACK_PAWN_A:
	case BLACK_PAWN_B:
	case BLACK_PAWN_C:
	case BLACK_PAWN_D:
	case BLACK_PAWN_E:
	case BLACK_PAWN_F:
	case BLACK_PAWN_G:
	case BLACK_PAWN_H:
	{
		return 1;
	}
	case WHITE_KNIGHT_A:
	case WHITE_KNIGHT_B:
	case BLACK_KNIGHT_A:
	case BLACK_KNIGHT_B:
	case WHITE_BISHOP_A:
	case WHITE_BISHOP_B:
	case BLACK_BISHOP_A:
	case BLACK_BISHOP_B:
	{
		return 3;
	}
	case WHITE_ROOK_A:
	case WHITE_ROOK_B:
	case BLACK_ROOK_A:
	case BLACK_ROOK_B:
	{
		return 5;
	}
	case WHITE_QUEEN:
	case BLACK_QUEEN:
	{
		return 9;
	}
	case WHITE_KING:
	case BLACK_KING:
	{
		if (!IsSpawned(piece))
		{
			return 0;
		}
		//count up friendly units within 1 tile and add the count to our power
		int power = 1;
		Vector2Int pos(GetPosition(piece));
		pos.y++;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.x++;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.y--;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.y--;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.x--;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.x--;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.y++;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		pos.y++;
		if (!CheckOOB(pos) && !IsEmpty(pos.Convert()) && !AreDifferentColor(piece, (PIECE_ORDER)CheckMailbox(pos.Convert())))
		{
			power++;
		}
		return power;
	}
	default:
		return 0;
	}
}
bool CChessEngine::IsInMoveBitBoard(char pos, PIECE_ORDER piece)
{
	return (PieceMoveBitBoards[(int)piece] & (1LL << (pos & POSITION_MASK)));
}
bool CChessEngine::IsInAttackBitBoard(char pos, PIECE_ORDER piece)
{
	return (PieceAttackBitBoards[(int)piece] & (1LL << (pos & POSITION_MASK)));
}

bool CChessEngine::IsInPressureBitBoard(char pos, PIECE_ORDER piece)
{
	return (OccupancyBitBoard & (1LL << (pos & POSITION_MASK)));
}

void CChessEngine::DespawnPiece(PIECE_ORDER piece)
{
	if (IsSpawned(piece))
	{
		OccupancyBitBoard &= ~(1LL << (PiecePositions[(int)piece] & POSITION_MASK));
		PiecePositions[(int)piece] = 0;
		RegenerateAllPieceBoards();
	}
}

bool CChessEngine::SpawnPiece(PIECE_ORDER piece, Vector2Int spawn)
{
	if (CheckOOB(spawn) || IsSpawned(piece) || !IsEmpty(spawn.Convert()))
	{
		return false;
	}
	//flag the spawn bits on the position
	PiecePositions[(int)piece] = 0;
	PiecePositions[(int)piece] |= SPAWN_MASK;
	PiecePositions[(int)piece] |= spawn.Convert();
	SetOccupied(spawn.Convert());
	//update mailbox
	Mailbox[(PiecePositions[(int)piece] & POSITION_MASK)] = piece;
	RegenerateAllPieceBoards();

	return true;
}

bool CChessEngine::TryMove(PIECE_ORDER piece, Vector2Int pos)
{
	if (CheckOOB(pos) || !(PiecePositions[(int)piece] & SPAWN_MASK) || !IsEmpty(pos.Convert()))
	{
		return false;
	}

	SetPosition(pos.Convert(), piece);
	return true;
}

#pragma endregion

#pragma endregion
