#include "Game.h"
#include "moveGenerator.h"

//set tile selected
void Game::setSelection(int index)
{
	lastTileSelected = index;
	//generate all possible move for the piece
	possibleMoveForPiece = moveGenerator::getLegalMoves(boardState, index);
	for (Move move : possibleMoveForPiece)
	{
		tilesToColor[move.to] = true;
	}
}

//clear tile selected
void Game::clearSelection()
{
	lastTileSelected = -1;
	for (int i = 0; i < 64; i++)
		tilesToColor[i] = false;
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
		{Piece::wK, "wK"}, {Piece::wQ, "wQ"}, {Piece::wR, "wR"}, {Piece::wB, "wB"}, {Piece::wN, "wN"}, {Piece::wP, "wP"},
		{Piece::bK, "bK"}, {Piece::bQ, "bQ"}, {Piece::bR, "bR"}, {Piece::bB, "bB"}, {Piece::bN, "bN"}, {Piece::bP, "bP"},
	};
	seePromotion = false;
	clearSelection();
	kingOnCheck = -1;

	//init boardstate
	this->boardState.board =
	{
		Piece::wR, Piece::wN, Piece::wB, Piece::wQ, Piece::wK, Piece::wB, Piece::wN, Piece::wR,
		Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP, Piece::wP,
		Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
		Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
		Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
		Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty, Piece::Empty,
		Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP, Piece::bP,
		Piece::bR, Piece::bN, Piece::bB, Piece::bQ, Piece::bK, Piece::bB, Piece::bN, Piece::bR
	};
	boardState.canCastle = { true, true, true, true };
	boardState.passantTarget = -1;
	boardState.whiteToMove = true;
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

void Game::setText(Colors color)
{
	switch (color)
	{
	case Colors::White:
		text->setString("White wins");
		break;
	case Colors::Black:
		text->setString("Black wins");
		break;
	case Colors::Draw:
		text->setString("Draw");
		break;
	case Colors::Continue:
		break;
	}
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
}

//Functions
const bool Game::running() const
{
	return this->window->isOpen();
}

//pool event and 
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
						std::array<Piece, 4> pieces = boardState.whiteToMove
							? std::array<Piece, 4>{ Piece::wQ, Piece::wR, Piece::wB, Piece::wN }
						: std::array<Piece, 4>{ Piece::bQ, Piece::bR, Piece::bB, Piece::bN };

						Piece piece = Piece::Empty;
						for (int i = 0; i < 4; i++)
						{
							int menuSquare = boardState.whiteToMove ? promotionMove.to - i * 8 : promotionMove.to + i * 8;
							if (index == menuSquare)
							{
								piece = pieces[i];
								break;
							}
						}
						//if a piece was selected
						if (piece != Piece::Empty)
						{
							promotionMove.promotionPiece = piece;
							moveGenerator::applyMove(boardState, promotionMove);
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
						if (isCurrentTurn(boardState.board[index], boardState.whiteToMove) && boardState.board[index] != Piece::Empty)
						{
							setSelection(index);
						}	
					}
					//if lastTileSelected != currentTileSelected
					else
					{
						bool validMove = false;
						Move rMove;
						//search if the move that you want to do is in the possibleMoveForPiece
						for (Move move : possibleMoveForPiece)
						{
							if (move.to == index)
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
							if (rMove.moveFlag == MoveFlag::Promotion)
							{
								seePromotion = true;
								promotionMove = rMove;
							}
							else
							{
								moveGenerator::applyMove(boardState, rMove);
								this->sound->play();
							}
						}
						//if the move isn't valid
						else
						{
							clearSelection();
							//if the new tile selected is of the same color
							if (isCurrentTurn(boardState.board[index], boardState.whiteToMove))
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
	if (moveGenerator::isSquareAttacked(boardState, moveGenerator::findKing(boardState, boardState.whiteToMove)))
		kingOnCheck = moveGenerator::findKing(boardState, boardState.whiteToMove);
	else
		kingOnCheck = -1;
}

//update
void Game::update()
{
	this->poolEvents();
	if (moveGenerator::getGameState(boardState) != Colors::Continue)
		setText(moveGenerator::getGameState(boardState));
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
		if (boardState.board[i] == Piece::Empty)
			continue;
		std::string key = pieceToKey[boardState.board[i]];
		sprites[key]->setPosition({ boardStartPos.x + (i % 8) * tileSize, boardStartPos.y + (7 - i / 8) * tileSize });
		this->window->draw(*sprites[key]);
	}
}

//render the promotion choice
void Game::renderPromotion(int square)
{
	std::array<Piece, 4> pieces;
	if (square / 8 == 7)
		pieces = { Piece::wQ, Piece::wR, Piece::wB, Piece::wN };
	else
		pieces = { Piece::bN, Piece::bB, Piece::bR, Piece::bQ };

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
	for (Piece piece : pieces)
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
		this->renderPromotion(promotionMove.to);

	this->window->draw(*text);

	this->window->display();
}