// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CHESSENGINE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CHESSENGINE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CHESSENGINE_EXPORTS
#define CHESSENGINE_API __declspec(dllexport)
#else
#define CHESSENGINE_API __declspec(dllimport)
#endif
#define BOARD_DIMENSIONS 8
#define PIECE_TOTAL 32
#define BOARD_SIZE 64
#define SPAWN_MASK (char)128
#define MOVED_MASK (char)64
#define POSITION_MASK (char)63

enum PIECE_ORDER
{
    WHITE_PAWN_A = 0,
    WHITE_PAWN_B,
    WHITE_PAWN_C,
    WHITE_PAWN_D,
    WHITE_PAWN_E,
    WHITE_PAWN_F,
    WHITE_PAWN_G,
    WHITE_PAWN_H,
    WHITE_ROOK_A,
    WHITE_ROOK_B,
    WHITE_KNIGHT_A,
    WHITE_KNIGHT_B,
    WHITE_BISHOP_A,
    WHITE_BISHOP_B,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN_A,
    BLACK_PAWN_B,
    BLACK_PAWN_C,
    BLACK_PAWN_D,
    BLACK_PAWN_E,
    BLACK_PAWN_F,
    BLACK_PAWN_G,
    BLACK_PAWN_H,
    BLACK_ROOK_A,
    BLACK_ROOK_B,
    BLACK_KNIGHT_A,
    BLACK_KNIGHT_B,
    BLACK_BISHOP_A,
    BLACK_BISHOP_B,
    BLACK_QUEEN,
    BLACK_KING,
    PIECE_ORDER_SIZE
};
struct Vector2Int
{
public:
    int x;
    int y;
    Vector2Int()
    {
        x = -1;
        y = -1;
    }
    
    Vector2Int(int _x, int _y)
    {
        x = _x;
        y = _y;
    }

    Vector2Int(char pos)
    {
        //just in case
        pos = pos & POSITION_MASK;
        x = pos % BOARD_DIMENSIONS;
        y = pos / BOARD_DIMENSIONS;
    }

    char Convert()
    {
        return (y * BOARD_DIMENSIONS) + x;
    }
    Vector2Int operator+(Vector2Int a) {
        return { a.x + x,a.y + y };
    }
    Vector2Int operator=(Vector2Int a) {
        x = a.x;
        y = a.y;
        return a;
    }
};
// This class is exported from the dll
class CHESSENGINE_API CChessEngine {
private:
    char PiecePositions[PIECE_TOTAL];
    long long PieceMoveBitBoards[PIECE_TOTAL];
    long long PieceAttackBitBoards[PIECE_TOTAL];
    long long PiecePressureBitBoards[PIECE_TOTAL];
    long long OccupancyBitBoard;
    long long EnPassantBitBoard;
    int PressureBoard[BOARD_SIZE];
    PIECE_ORDER Mailbox[BOARD_SIZE];
    PIECE_ORDER EnPassantPiece;

    void SetOccupied(char pos);
    void SetUnoccupied(char pos);
    bool IsSpawned(PIECE_ORDER piece);
    bool HasMoved(PIECE_ORDER piece);
    bool AreDifferentColor(PIECE_ORDER pieceOne, PIECE_ORDER pieceTwo);
    bool CheckOOB(Vector2Int pos);
    bool CheckOOB(char pos);
    char GetPosition(PIECE_ORDER piece);
    bool IsPawn(PIECE_ORDER piece);
    bool EmptyAndInBounds(Vector2Int tile);
    bool PawnAttackCheck(Vector2Int pos, PIECE_ORDER piece);
    bool HasEnemyAndInBounds(Vector2Int pos, PIECE_ORDER attackingPiece);

    void SetPosition(char position, PIECE_ORDER piece);
    void RegenerateAllPieceBoards();
    void GenerateAttackAndMoveBoards(PIECE_ORDER piece);
public:
	CChessEngine();
	virtual ~CChessEngine();

	// TODO: add your methods here.
    int GetPiecePositionInt(PIECE_ORDER piece);
    int CheckMailbox(char pos);
    bool IsEnPassantTile(char pos);
    bool IsEmpty(char pos);
    int GetPressure(char pos);
    int GetValue(PIECE_ORDER piece);
    bool IsInMoveBitBoard(char pos, PIECE_ORDER piece);
    bool IsInAttackBitBoard(char pos, PIECE_ORDER piece);
    bool IsInPressureBitBoard(char pos, PIECE_ORDER piece);
    void DespawnPiece(PIECE_ORDER piece);
    bool SpawnPiece(PIECE_ORDER piece, Vector2Int spawn);
    bool TryMove(PIECE_ORDER piece, Vector2Int pos);
};


extern "C" CHESSENGINE_API CChessEngine* CreateChessEngineClass();
extern "C" CHESSENGINE_API void DisposeChessEngineClass(CChessEngine* pObject);
extern "C" CHESSENGINE_API int GetPiecePosition(CChessEngine* pObject, int piece);
extern "C" CHESSENGINE_API int CheckMailbox(CChessEngine* pObject, char pos);
extern "C" CHESSENGINE_API bool IsEnPassantTile(CChessEngine* pObject, char pos);
extern "C" CHESSENGINE_API bool IsEmpty(CChessEngine* pObject, char pos);
extern "C" CHESSENGINE_API int GetPressure(CChessEngine* pObject, char pos);
extern "C" CHESSENGINE_API int GetValue(CChessEngine* pObject, int piece);
extern "C" CHESSENGINE_API bool IsInMoveBitBoard(CChessEngine* pObject, char pos, int piece);
extern "C" CHESSENGINE_API bool IsInAttackBitBoard(CChessEngine* pObject, char pos, int piece);
extern "C" CHESSENGINE_API bool IsInPressureBitBoard(CChessEngine* pObject, char pos, int piece);
extern "C" CHESSENGINE_API void DespawnPiece(CChessEngine* pObject, int piece);
extern "C" CHESSENGINE_API bool SpawnPiece(CChessEngine* pObject, int piece, char spawn);
extern "C" CHESSENGINE_API bool TryMove(CChessEngine* pObject, int piece, char pos);