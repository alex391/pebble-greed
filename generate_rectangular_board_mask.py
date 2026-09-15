from generate_circular_board_mask import (
    print_board_mask,
    CHARACTER_WIDTH,
    CHARACTER_HEIGHT,
    BOARD_WIDTH,
    BOARD_HEIGHT,
)

SCREEN_WIDTH = 144
SCREEN_HEIGHT = 168

def main():
    drawable_board_width = SCREEN_WIDTH // CHARACTER_WIDTH
    drawable_board_height = SCREEN_HEIGHT // CHARACTER_HEIGHT

    board_mask = []
    for y in range(BOARD_HEIGHT):
        if y < drawable_board_height:
            board_mask_row = []
            for x in range(BOARD_WIDTH):
                board_mask_row.append(1 if x < drawable_board_width else 0)
        else: 
            board_mask_row = [0 for _ in range(BOARD_WIDTH)]
        board_mask.append(board_mask_row)

    print_board_mask(board_mask)


if __name__ == "__main__":
    main()
