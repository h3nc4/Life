// test_engine.c
#include <stdio.h>
#include <assert.h>
#include "engine.h"

void test_initialize_game()
{
	initialize_game(5, 5);
	u8int **grids = ptr_to_both_grids();
	for (unsigned int i = 0; i < 25; i++)
	    assert(grids[0][i] == 0 || grids[0][i] == 1);
}

void test_toggle_cell_state()
{
	initialize_game(3, 3);
	u8int **grids = ptr_to_both_grids();
	unsigned int y = 1, x = 1;
	assert(grids[0][y * 3 + x] == 0 || grids[0][y * 3 + x] == 1);
	u8int val = grids[0][y * 3 + x];
	toggle_cell_state(y, x);
	assert(grids[0][y * 3 + x] == val^1);
	toggle_cell_state(y, x);
	assert(grids[0][y * 3 + x] == val);
}

void test_update_grid()
{
	initialize_game(3, 3);
	u8int **grids = ptr_to_both_grids();
	clear_grid();
	grids[0][1 * 3 + 1] = 1;
	grids[0][1 * 3 + 2] = 1;
	update_grid();
	assert(grids[0][1 * 3 + 1] == 0);
}

int main()
{
	test_initialize_game();
	test_toggle_cell_state();
	test_update_grid();
	printf("All tests passed!\n");
	return 0;
}
