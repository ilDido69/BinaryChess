#include "gui.h"
#include "moveGen.h"

//set tile selected
void Game::setSelection(int index)
{
	lastTileSelected = index;
	//generate all possible move for the piece
	MoveGen::getLegalMoves(boardState, legalMoves);
	for (int i = 0; i < legalMoves.count; i++)
	{
		Move move = legalMoves.moves[i];
		if (getFrom(move) == index)
			tilesToColor[getTo(move)] = true;
	}
}

//clear tile selected
void Game::clearSelection()
{
	lastTileSelected = -1;
	for (int i = 0; i < 64; i++)
		tilesToColor[i] = false;
	legalMoves.count = 0;
}

//initialize all the variables
void Game::initVariables()
{
	this->endGame = false;
	this->tileSize = 80.f;
	this->windowSize = { 1200, 900 };
	this->boardStartPos = { (windowSize.x - tileSize * 8) / 2, (windowSize.y - tileSize * 8) / 2 };
	pieceToKey =
	{
		{wK, "wK"}, {wQ, "wQ"}, {wR, "wR"}, {wB, "wB"}, {wN, "wN"}, {wP, "wP"},
		{bK, "bK"}, {bQ, "bQ"}, {bR, "bR"}, {bB, "bB"}, {bN, "bN"}, {bP, "bP"},
	};
	seePromotion = false;
	clearSelection();
	kingOnCheck = -1;

	//init boardstate
	MoveGen::resetBoardState(boardState);
}

//init the window
void Game::initWindow()
{
	this->videoMode = sf::VideoMode(windowSize);
	//this->videoMode = sf::VideoMode::getDesktopMode;
	this->window = new sf::RenderWindow(this->videoMode, "Game", sf::Style::Close | sf::Style::Titlebar);
	this->window->setFramerateLimit(60);
}

//the function initializes 12 textures and 12 sprites (one for each piece)
void Game::initTexturesAndSprites()
{
	std::vector<std::string> pieces = {
		"wK", "wQ", "wR", "wB", "wN", "wP",
		"bK", "bQ", "bR", "bB", "bN", "bP",
	};

	for (std::string piece : pieces)
	{
		sf::Texture t;
		if (!t.loadFromFile("assets/images/" + piece + ".png"))
			std::cout << "Error, couldn't load " << piece << ".png" << std::endl;
		textures[piece] = std::move(t);
		textures[piece].setSmooth(true);
		sprites[piece] = new sf::Sprite(textures[piece]);
		sprites[piece]->setScale({ tileSize / textures[piece].getSize().x, tileSize / textures[piece].getSize().y});
	}
}

void Game::setText(int endText)
{
	if (endText == 0)
		text->setString("White wins");
	else if (endText == 1)
		text->setString("Black wins");
	else if (endText == 2)
		text->setString("Draw");
	else
		return;

	sf::FloatRect textBounds = text->getLocalBounds();

	text->setOrigin({ textBounds.position.x + textBounds.size.x / 2.f, textBounds.position.y + textBounds.size.y / 2.f });

	text->setPosition({ window->getSize().x / 2.f, 50 });
}

//Constructors and Destructors
Game::Game() : font(sf::Font("assets/fonts/arial.ttf")), text(sf::Text(*font, "", 24)), soundBuffer(nullptr), sound(nullptr) 
{
	soundBuffer = new sf::SoundBuffer();
	sound = new sf::Sound(*soundBuffer);
	if (!soundBuffer->loadFromFile("assets/sounds/chessmove.wav")) {
		std::cout << "Error, couldn't load chessmove.wav" << std::endl;
	}
	sound->setBuffer(*soundBuffer);

	this->initVariables();
	this->initWindow();
	this->initTexturesAndSprites();
}

Game::~Game()
{
	delete this->window;
	for (auto& [key, sprite] : sprites)
		delete sprite;

	delete this->sound;
	delete this->soundBuffer;
}

//Functions
const bool Game::running() const
{
	return this->window->isOpen();
}

//pool event
void Game::poolEvents()
{
	while (auto sfmlEvent = this->window->pollEvent()) {
		if (sfmlEvent->is<sf::Event::Closed>())
			this->window->close();
		

		if (auto* click = sfmlEvent->getIf<sf::Event::MouseButtonPressed>())
		{
			//if left button is pressed
			if (click->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i mousePos = click->position;
				float colF = ((float)mousePos.x - boardStartPos.x) / tileSize;
				float rowF = ((float)mousePos.y - boardStartPos.y) / tileSize;
				int col = (int)std::floor(colF);
				int row = (int)std::floor(rowF);

				if (col >= 0 && col < 8 && row >= 0 && row < 8)
				{
					int index = col + (7 - row) * 8;

					//if promotion choice
					if (seePromotion)
					{
						std::array<GuiPiece, 4> pieces = boardState.sideToMove
							? std::array<GuiPiece, 4>{ wQ, wR, wB, wN }
							: std::array<GuiPiece, 4>{ bQ, bR, bB, bN };

						GuiPiece piece = Empty;
						for (int i = 0; i < 4; i++)
						{
							int menuSquare = boardState.sideToMove == WHITE ? getTo(promotionMove) - i * 8 : getTo(promotionMove) + i * 8;
							if (index == menuSquare)
							{
								piece = pieces[i];
								break;
							}
						}
						//if a piece was selected
						if (piece != Empty)
						{
							Piece translatedPiece;
							switch (piece)
							{
							case wQ:
							case bQ:
								translatedPiece = QUEEN;
								break;

							case wR:
							case bR:
								translatedPiece = ROOK;
								break;

							case wB:
							case bB:
								translatedPiece = BISHOP;
								break;

							case wN:
							case bN:
								translatedPiece = KNIGHT;
								break;
							}
							
							promotionMove = encodeMove(getFrom(promotionMove), getTo(promotionMove), PAWN, getFlag(promotionMove), translatedPiece);
							MoveGen::makeMove(boardState, promotionMove, saved);
							legalMoves.count = 0;
							this->sound->play();
							seePromotion = false;
							continue;
						}
						else
							seePromotion = false;
					}
					//if the same tile is selected
					if (lastTileSelected == index)
					{
						clearSelection();
					}
					//if there isn't lastTileSelected
					else if (lastTileSelected == -1)
					{
						//if valid tile
						if (boardState.sideToMove == WHITE)
						{
							if (boardState.byColor[WHITE] & (1ULL << index))
								setSelection(index);
						}
						else if (boardState.byColor[BLACK] & (1ULL << index))
							setSelection(index);
					}
					//if lastTileSelected != currentTileSelected
					else
					{
						bool validMove = false;
						Move rMove;
						//search if the move that you want to do is in the possibleMoveForPiece
						for (int i = 0; i < legalMoves.count; i++)
						{
							Move move = legalMoves.moves[i];
							if (getTo(move) == index && getFrom(move) == lastTileSelected)
							{
								rMove = move;
								validMove = true;
								break;
							}
						}
						//if the move is valid
						if (validMove)
						{
							clearSelection();
							if (getFlag(rMove) == MoveFlag::PROMO || getFlag(rMove) == MoveFlag::PROMO_CAP)
							{
								seePromotion = true;
								promotionMove = rMove;
							}
							else
							{
								MoveGen::makeMove(boardState, rMove, saved);
								legalMoves.count = 0;
								this->sound->play();
							}
						}
						//if the move isn't valid
						else
						{
							clearSelection();
							//if the new tile selected is of the same color
							if (boardState.sideToMove == WHITE)
							{
								if (boardState.byColor[WHITE] & (1ULL << index))
									setSelection(index);
							}
							else if (boardState.byColor[BLACK] & (1ULL << index))
								setSelection(index);
						}
					}
				}
			}
		}
	}
}

//updates check gui (if the king is on check -> his square became red)
void Game::updateChecks(BoardState& boardState)
{
	if (MoveGen::onCheck(boardState))
		kingOnCheck = MoveGen::findKing(boardState, boardState.sideToMove);
	else
		kingOnCheck = -1;
}

//update
void Game::update()
{
	this->poolEvents();
	setText(MoveGen::getGameState(boardState));
	this->updateChecks(boardState);
}

//render the board
void Game::renderTable()
{
	for (int row = 0; row < 8; row++) 
	{
		for (int col = 0; col < 8; col++)
		{
			sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
			rect.setPosition(sf::Vector2f(boardStartPos.x + tileSize * col, boardStartPos.y + tileSize * row));

			if (!tilesToColor[(7- row) * 8 + col] && !(lastTileSelected == (7 - row) * 8 + col) && !(kingOnCheck == (7 - row) * 8 + col))
			{
				if ((row + col) % 2 == 0)
					rect.setFillColor(sf::Color(240, 217, 181, 255));
				else
					rect.setFillColor(sf::Color(181, 136, 99, 255));
			}
			else if (lastTileSelected == (7 - row) * 8 + col && !(kingOnCheck == (7 - row) * 8 + col))
			{
				if ((row + col) % 2 == 0)
					rect.setFillColor(sf::Color(120, 170, 255, 140));
				else
					rect.setFillColor(sf::Color(80, 120, 200, 170));
			} 
			else if (!(kingOnCheck == (7 - row) * 8 + col))
			{
				if ((row + col) % 2 == 0)
					rect.setFillColor(sf::Color(90, 220, 140, 140));
				else
					rect.setFillColor(sf::Color(20, 140, 80, 200));
			}
			else
			{
				if ((row + col) % 2 == 0)
					rect.setFillColor(sf::Color(242, 128, 128, 255));
				else
					rect.setFillColor(sf::Color(191, 64, 64, 255));
			}
			this->window->draw(rect);
		}
	}
}

//render the pieces of the current position
void Game::renderPieces()
{
	for (int i = 0; i < 64; i++) 
	{
		if (!(boardState.occupied & (1ULL << i)))
			continue;
		std::string key;
		Piece piece;
		if (boardState.byColor[WHITE] & (1ULL << i))
		{
			key = "w";
			piece = getPiece(boardState, WHITE, i);
		}
		else
		{
			key = "b";
			piece = getPiece(boardState, BLACK, i);
		}
		switch (piece)
		{
		case PAWN:
			key += "P";
			break;
		case KNIGHT:
			key += "N";
			break;
		case BISHOP:
			key += "B";
			break;
		case ROOK:
			key += "R";
			break;
		case QUEEN:
			key += "Q";
			break;
		case KING:
			key += "K";
			break;
		}
		sprites[key]->setPosition({ boardStartPos.x + (i % 8) * tileSize, boardStartPos.y + (7 - i / 8) * tileSize });
		this->window->draw(*sprites[key]);
	}
}

//render the promotion choice
void Game::renderPromotion(int square)
{
	std::array<GuiPiece, 4> pieces;
	if (square / 8 == 7)
		pieces = { wQ, wR, wB, wN };
	else
		pieces = { bN, bB, bR, bQ };

	sf::Vector2f startPosition = { boardStartPos.x + tileSize * (square % 8), (square / 8 == 7) ? boardStartPos.y : boardStartPos.y + tileSize * 4 };

	sf::RectangleShape rect;
	rect.setSize({ tileSize, tileSize });
	for (int i = 0; i < 4; i++)
	{
		rect.setFillColor(i % 2 == 0 ? sf::Color(222, 205, 171, 255) : sf::Color(235, 200, 120, 255));
		rect.setPosition({ startPosition.x, startPosition.y + tileSize * i });
		this->window->draw(rect);
	}
	int i = 0;
	for (GuiPiece piece : pieces)
	{
		std::string key = pieceToKey[piece];
		sprites[key]->setPosition({ startPosition.x, startPosition.y + i * tileSize });
		i++;
		this->window->draw(*sprites[key]);
	}
}

//render
void Game::render()
{
	this->window->clear();

	this->renderTable();
	this->renderPieces();
	if (seePromotion)
		this->renderPromotion(getTo(promotionMove));

	this->window->draw(*text);

	this->window->display();
}