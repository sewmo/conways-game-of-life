# Description
An implementation of the classic John Conway's Game of Life. The project is built in C with the SDL2 framework.  
John Conway's Game of Life is a cellular automaton created by the British mathematician John Horton Conway in 1970.  
The Game of Life is known as a zero-player game as the evolution of the game is determined only by the initial configuration of the player, requiring no further input.  
The Game of Life evolves in discrete time steps known as generations. The 2 dimensional grid is comprised of cells that can be found in one of two possible states: dead or live.  
Each cell interacts with its eight neighbouring cells, following a ruleset that determines its transition once per every generation.  

# Ruleset
The following rules are applied to every cell in the two dimensional grid:  
  1. If the cell is live has fewer than two live neighbours, it dies as if by underpopulation.  
  2. If the cell is live and has two or three live nighbours, it lives on to the next generation.  
  3. If the cell is live and has more than three live neighbours, it dies as if by overpopulation.  
  4. If the cell is dead and has exactly three live neighbours, it becomes a live cell, as if by reproduction.  

# Specifications
This project was built with the **Simple DirectMedia Layer** framework (SDL2).  

# Dependencies
**SDL2**     - Simple DirectMedia Layer framework for graphics rendering.  
**SDL2_ttf** - For TTF font loading and rendering.  

# Controls
**Space Bar:** Start Simulation / Stop Simulation  
**↑:** Increase update delta time (+5 snap)
**↓:** Decrease update delta time (-5 snap)  
**Middle Mouse Button:** Clears the grid  
**Click:** Spawn a cell at the mouse cursor  

# Installation
Download the .ZIP file, extract it, and run the 'main' executable.  

# Preview
<img width="596" height="616" alt="Screenshot 2026-04-26 154042" src="https://github.com/user-attachments/assets/841dd047-bc65-4760-be91-2212cc7791ef" />
 
