//
// Created by user on 3/10/2018.
//

#include <ctype.h>
#include "SPGUIGameScreen.h"
#include "SPMiniMax.h"


SDL_Texture* pieces[12] ;


SP_GUI_MESSAGE drawGameWindow(int* settings){

    SP_GUI_MESSAGE ret ;
    ChessBoard* board ;

    // create main SDL window
    SDL_Window *window = SDL_CreateWindow(
            "SPChess",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1270,
            768,
            SDL_WINDOW_OPENGL);

    // make sure window was created successfully
    if (window == NULL) {
        printf("ERROR: unable to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return ERROR;
    }

    // create a renderer for the window
    SDL_Renderer *rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (rend == NULL) {
        printf("ERROR: unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ERROR;
    }

    // ensure renderer supports transparency
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

    // set background
    SDL_Surface* surface = SDL_LoadBMP("bmp/game/bg.bmp") ;
    if (surface==NULL){
        printf("ERROR: unable to load image: %s\n", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        return ERROR ;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface) ;
    if (texture==NULL){
        printf("ERROR: unable to create texture from image: %s\n", SDL_GetError());
        SDL_FreeSurface(surface) ;
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        return ERROR ;
    }
    SDL_FreeSurface(surface) ;
    SDL_RenderCopy(rend, texture, NULL, NULL);

    // create piece textures from image
    ret = createPieceTextures("bmp/game/pieces.bmp", rend) ;
    if (ret!=NONE){
        printf("ERROR: unable to create piece textures: %s\n", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        return ERROR ;
    }

    // Create board
    board = createGUIChessGame(rend, "bmp/game/brightSquare.bmp", "bmp/game/darkSquare.bmp", settings) ;
    if (board==NULL){
        printf("ERROR: unable to create board: %s\n", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        return ERROR ;
    }

    // Event loop
    while(1){
        drawBoard(rend, board) ;
        SDL_RenderPresent(rend);

        SDL_Event e ;
        Button* button ;
        SDL_WaitEvent(&e) ;
        if (e.type==SDL_QUIT||e.key.keysym.sym==SDLK_ESCAPE)
            break ;
        ret = handleBoardEvent(board, &e) ;
        // Add buttons event handling 
        if (ret==QUIT||ret==ERROR||ret==MAIN_MENU||ret==START_GAME)
            break ;

        drawBoard(rend, board) ;
        SDL_RenderPresent(rend);

        // Check for computer turn and execute it
        if ((board->game->gameMode==1)&&((board->game->currentPlayer==SP_CHESS_GAME_WHITE_SYMBOL&&board->game->userColor==0)||
                                         (board->game->currentPlayer==SP_CHESS_GAME_BLACK_SYMBOL&&board->game->userColor==1))){
            int maxDepth = board->game->difficulty ;
            SPMove* move = spMinimaxSuggestMove(board->game, maxDepth) ;
            if (move==NULL)
                return ERROR ;
            executeGUIMove(board, move) ;
            continue ;
        }
    }

    //destroyGUIGame() ;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);

    return ret ;
}


ChessBoard* createGUIChessGame(SDL_Renderer* rend, char* brightSquareImg, char* darkSquareImg,
                               int* settings){
    
    ChessBoard* board = (ChessBoard*)malloc(sizeof(ChessBoard)) ;
    SPChessGame* game = spChessCreate(settings) ;
    SDL_Rect* location = (SDL_Rect*)malloc(sizeof(SDL_Rect)) ;
    if (board==NULL||game==NULL||location==NULL)
        return NULL ;
    
    for (int i=0;i<SP_CHESS_GAME_N_ROWS;i++){
        for (int j=0;j<SP_CHESS_GAME_N_COLUMNS;j++){
            SDL_Rect* loc = (SDL_Rect*)malloc(sizeof(SDL_Rect)) ;
            if (loc==NULL)
                return NULL ;
            loc->x = BOARD_X + j*SQUARE_SIDE ;
            loc->y = BOARD_Y + i*SQUARE_SIDE ;
            loc->w = SQUARE_SIDE ;
            loc->h = SQUARE_SIDE ;
            if ((i+j)%2==0)
                board->squares[i][j] = createChessSquare(rend, brightSquareImg, loc, getPieceTex(game->gameBoard[i][j])) ;
            else
                board->squares[i][j] = createChessSquare(rend, darkSquareImg, loc, getPieceTex(game->gameBoard[i][j])) ;
            if (board->squares[i][j]==NULL)
                return NULL ;

        }
    }
    
    location->x = BOARD_X ;
    location->y = BOARD_Y ;
    location->h = SQUARE_SIDE*SP_CHESS_GAME_N_ROWS ;
    location->w = SQUARE_SIDE*SP_CHESS_GAME_N_COLUMNS ;
    
    board->location = location ;
    board->game = game ;
    board->pressed = false ;

    return board ;
}


void drawBoard(SDL_Renderer* rend, ChessBoard* board){
    for (int i=0;i<SP_CHESS_GAME_N_ROWS;i++) {
        for (int j = 0; j < SP_CHESS_GAME_N_COLUMNS; j++) {
            board->squares[i][j]->draw(board->squares[i][j], rend) ;
        }
    }
}


SP_GUI_MESSAGE createPieceTextures(char* piecesImg, SDL_Renderer* rend){

    SDL_Rect* rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    if (rect==NULL)
        return  ERROR ;
    // load an image onto an SDL surface
    SDL_Surface* surf = SDL_LoadBMP(piecesImg);
    if (surf == NULL) {
        printf("ERROR: unable to load image: %s\n", SDL_GetError());
        free(rect) ;
        return ERROR;
    }
    // set a specific color (white: 255,255,255) to transparent
    SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 255, 255, 255));
    // create a texture from the surface image
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
    if (tex == NULL) {
        printf("ERROR: unable to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surf);
        free(rect) ;
        return ERROR ;
    }

    // surface not needed anymore - free it
    SDL_FreeSurface(surf);

    for (int i=0;i<12;i++){
        rect->x = PIECE_WIDTH*(i%6) ;
        rect->y = PIECE_HEIGHT*(i/6) ;
        rect->h = PIECE_HEIGHT ;
        rect->w = PIECE_WIDTH ;
        pieces[i] = SDL_CreateTexture(rend,SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, SQUARE_SIDE, SQUARE_SIDE) ;
        SDL_SetTextureBlendMode(pieces[i], SDL_BLENDMODE_BLEND);
        if (pieces[i] == NULL) {
            printf("ERROR: unable to create piece texture: %s\n", SDL_GetError());
            free(rect) ;
            return ERROR;
        }

        SDL_SetRenderTarget(rend, pieces[i]);
        SDL_RenderCopy(rend, tex, rect, NULL);
    }
    free(rect) ;
    SDL_SetRenderTarget(rend, NULL);
    return NONE ;
}


SDL_Texture* getPieceTex(char piece){

    int white ;
    int index ;

    if (isupper(piece))
        white = 0 ;
    else
        white = 1 ;

    switch (tolower(piece)){
        case 'r':
            index = 0 ;
            break ;
        case 'b':
            index = 1 ;
            break ;
        case 'q':
            index = 2 ;
            break ;
        case 'k':
            index = 3 ;
            break ;
        case 'n':
            index = 4 ;
            break ;
        case 'm':
            index = 5 ;
            break ;
        default:
            return NULL ;
    }

    index += white*6 ;
    return pieces[index] ;
}



SP_GUI_MESSAGE handleBoardEvent(ChessBoard* board, SDL_Event* e){
    // Check if the event is mouse event, and if it's in the board
    if (e->type!=SDL_MOUSEMOTION&&e->type!=SDL_MOUSEBUTTONDOWN&&e->type!=SDL_MOUSEBUTTONUP)
        return NONE ;

    SP_GUI_MESSAGE ret ;
    SDL_Point p = {0, 0};
    SDL_GetMouseState(&p.x, &p.y) ;
    int squareCol = (p.x - BOARD_X)/SQUARE_SIDE ;
    int squareRow = (p.y - BOARD_Y)/SQUARE_SIDE ;

    ChessSquare* square ;
    if (e->type==SDL_MOUSEMOTION){
        for (int i=0;i<SP_CHESS_GAME_N_ROWS;i++){
            for (int j=0;j<SP_CHESS_GAME_N_COLUMNS;j++) {
                square = (ChessSquare*)board->squares[i][j]->data ;
                if (SDL_PointInRect(&p, square->location)&&!square->hover&&square->piece!=NULL){
                    square->hover = true ;
                }
                if (!SDL_PointInRect(&p, square->location)&&square->hover){
                    square->hover = false ;
                }
            }
        }
    }

    if(e->type==SDL_MOUSEBUTTONUP){
        // if click is out of board clear highlighted buttons and return
        if (!SDL_PointInRect(&p, board->location)){
            clearBoard(board) ;
            return NONE ;
        }
        square = (ChessSquare*)board->squares[squareRow][squareCol]->data ;

        // if click is on highlighted square(possible move), execute the move
        if (board->pressed&&square->highlighted){
            SPMove* move = (SPMove*)malloc(sizeof(SPMove)) ;
            if (move==NULL)
                return ERROR ;
            move->sourceRow = 7 - board->pressedLocation[0] ;
            move->sourceColumn = board->pressedLocation[1] ;
            move->destRow = 7 - squareRow ;
            move->destColumn = squareCol ;
            executeGUIMove(board, move) ;
            clearBoard(board) ;
            free(move) ;
            return NONE ;
        }

        clearBoard(board) ;
        board->pressed = true ;
        board->pressedLocation[0] = squareRow ;
        board->pressedLocation[1] = squareCol ;
        ret = colorPossibleMoves(board) ;
        return ret ;
    }
    return NONE ;
}


void clearBoard(ChessBoard* board){
    ChessSquare* square ;
    for (int i=0;i<SP_CHESS_GAME_N_ROWS;i++){
        for (int j=0;j<SP_CHESS_GAME_N_COLUMNS;j++) {
            square = (ChessSquare*)board->squares[i][j]->data ;
            square->pressed = false ;
            square->highlighted = false ;
            square->threatend = false ;
            square->capture = false ;
        }
    }
    board->pressed = false ;
}


void executeGUIMove(ChessBoard* board, SPMove* move){

    ChessSquare* dest = (ChessSquare*)board->squares[7-move->destRow][move->destColumn]->data ;
    ChessSquare* src = (ChessSquare*)board->squares[7-move->sourceRow][move->sourceColumn]->data ;

    executeMove(board->game, move) ;

    // move piece texture from source to dest square
    dest->piece = src->piece ;
    src->piece = NULL ;
}


SP_GUI_MESSAGE colorPossibleMoves(ChessBoard* board){

    SPArrayList* moves = spChessGetMoves(board->game, 7 - board->pressedLocation[0], board->pressedLocation[1]) ;
    if (moves==NULL)
        return ERROR ;

    // check if the piece belongs to the current player
    char piece = board->game->gameBoard[7-board->pressedLocation[0]][board->pressedLocation[1]] ;
    char player = board->game->currentPlayer ;
    if ((board->game->currentPlayer==SP_CHESS_GAME_WHITE_SYMBOL&&isupper(piece))||
            (board->game->currentPlayer==SP_CHESS_GAME_BLACK_SYMBOL&&islower(piece)))
        return NONE ;


    SPMove* move ;
    ChessSquare* square ;

    for (int i=0;i<moves->actualSize; i++){
        move = spArrayListGetAt(moves, i) ;
        square = (ChessSquare*)board->squares[7-move->destRow][move->destColumn]->data ;
        square->highlighted = true ;

        if (square->piece!=NULL)
            square->capture = true ;

        testMove(board->game, move) ;
        if (spChessIsThreatend(board->game, move->destRow, move->destColumn, player)==SP_CHESS_GAME_UNDER_THREAT)
            square->threatend = true ;
        undoTestMove(board->game, move) ;
    }

    return NONE ;
}