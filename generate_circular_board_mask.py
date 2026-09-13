import math

BOARD_WIDTH = 20
BOARD_HEIGHT = 17
SCREEN_DIAMETER = 260
CHARACTER_WIDTH = 13
CHARACTER_HEIGHT = 16


def board_coordinate_to_screen_box(x: int, y: int) -> list[tuple[int, int]]:
    x = x * CHARACTER_WIDTH
    y = y * CHARACTER_HEIGHT

    return [
        (x, y),
        (x + CHARACTER_WIDTH, y),
        (x, y + CHARACTER_HEIGHT),
        (x + CHARACTER_WIDTH, y + CHARACTER_HEIGHT),
    ]


def distance(p1, p2, q1, q2):
    return math.hypot(p1 - q1, p2 - q2)


def on_screen(x, y) -> bool:
    screen_radius = SCREEN_DIAMETER // 2
    return distance(x, y, screen_radius, screen_radius) <= screen_radius


def main():
    board_mask = []
    for y_board in range(BOARD_HEIGHT):
        board_mask_row = []
        for x_board in range(BOARD_WIDTH):
            box = board_coordinate_to_screen_box(x_board, y_board)
            mask = 1
            for coordinate in box:
                if not on_screen(coordinate[0], coordinate[1]):
                    mask = 0
                    break
            board_mask_row.append(mask)
        board_mask.append(board_mask_row)


    print("{")
    for j, row in enumerate(board_mask):
        print("  { ", end="")
        for i, x in enumerate(row):
            if i == len(row) - 1:
                print(f"{x}", end="")
            else:
                print(f"{x}, ", end="")
        if j == len(board_mask) - 1:
            print(" }")
        else:
            print(" },")

    print("};")
    


if __name__ == "__main__":
    main()
