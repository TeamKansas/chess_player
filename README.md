# chess_player
This is a chess player. There is no bot, you can play against yourself or with a friend. It prints the board on the terminal, so put terminal in big screen for it to work. If your computer does not support colored text in the terminal, it won't print properly.

Instructions:
Red player starts in "piece select mode".
Use 'w', 's', 'a', or 'd' to move the cursor. The cursor is a green square surrounding the tile it is on.
Press Space to select the piece you wish to move. The program will then go into "move select mode".
Move the cursor to the tile you wish to place the selected piece. In "move select mode" the active piece will be highlighted along with the cursor.

Press enter key to move the selected piece to tile currently occupied by the cursor, ending the turn and moving on to the next player's turn.
If you decide you do not want to move the currently selected piece, press 'q', this will put the program back into piece select mode. 
The program does not allow illegal moves. If an illegal move is attempted, nothing will happen. Either select a different piece or choose another tile.

Note: Castling is a ROOK's move in this program, not a King's move. Select the rook by pressing space, then move the cursor to the King and press enter.
This will perform the castling move if it is legal to do so.

After a move is played, the program will wait approximately 0.6 seconds before flipping the board and beginning the opponents turn.
