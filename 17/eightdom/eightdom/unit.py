def default(cursor, move):
    self = cursor.get()
    target = cursor.get(*move)

    # wall
    if target[1] >= 10:
        return

    # auto-select
    if self[2] == 0 and target[2] == cursor.owner:
        cursor.apply(move)
        cursor.repick()
        return

    # forward-full
    if target[2] != 0:
        cursor.apply(move)

    # forward-empty
    while target[1:3] == (0, 0):
        cursor.apply(move)
        target = cursor.get(*move)


def one(cursor, move):
    self = cursor.get()
    target = cursor.get(*move)

    # wall
    if target[1] >= 10:
        return

    # fusion
    if target == self:
        cnew = (str(target[1] + 1), target[1] + 1, target[2])

        cursor.set((' ', 0, 0))
        cursor.set(cnew, *move)

        cursor.apply(move)
        cursor.repick()

        if cursor.owner == 1:
            cursor.world.moves -= 1
            cursor.world.score += 20

        return

    # push
    if target[2] == cursor.owner and target[1] <= (self[1] + 1):
        p = cursor.pos
        cursor.apply(move)
        pick(target[1])(cursor, move)
        cursor.pos = p

        cursor.unit = default
        cursor.apply(move)

        if cursor.owner == 1:
            cursor.world.moves -= 1
            cursor.world.score += 5

        return

    # blocked (by bigger objects)
    if target[2] == cursor.owner:
        cursor.unit = default
        cursor.apply(move)

        if cursor.owner == 1:
            cursor.world.moves += 2

        return

    # kill equal
    if target[2] != cursor.owner and target[2] > 0 and target[1] == self[1]:
        cursor.set((' ', 0, 0), *move)
        cursor.set((' ', 0, 0))

        c = cursor.world.get_cursor(target[2])
        if (c.pos[0] == cursor.pos[0] + move[0] and
                c.pos[1] == cursor.pos[1] + move[1]):
            c.repick()

        cursor.apply(move)
        cursor.repick()

        if cursor.owner == 1:
            cursor.world.moves -= 10
            cursor.world.score += 50

        return

    # kill inferiors
    if (target[2] != cursor.owner and target[2] > 0 and
            target[1] <= self[1] - 1):
        if target[1] == self[1] - 1:
            cnew = (str(self[1] - 1), self[1] - 1, self[2])
        else:
            cnew = cursor.get()

        cursor.set(cnew, *move)
        cursor.set((' ', 0, 0))

        c = cursor.world.get_cursor(target[2])
        if (c.pos[0] == cursor.pos[0] + move[0] and
                c.pos[1] == cursor.pos[1] + move[1]):
            c.repick()

        cursor.apply(move)

        if cursor.owner == 1:
            cursor.world.moves -= 1
            cursor.world.score += 30

        return

    # attack superiors
    if (target[2] != cursor.owner and target[2] > 0 and
            target[1] >= self[1] + 1):
        if target[1] == self[1] + 1:
            cnew = (str(target[1] - 1), target[1] - 1, target[2])
        else:
            cnew = cursor.get(*move)

        cursor.set(cnew, *move)
        cursor.set((' ', 0, 0))

        c = cursor.world.get_cursor(target[2])
        if (c.pos[0] == cursor.pos[0] + move[0] and
                c.pos[1] == cursor.pos[1] + move[1]):
            c.repick()

        cursor.repick()
        cursor.apply(move)
        return

    # normal move
    if target == (' ', 0, 0):
        cursor.set(cursor.get(), *move)
        cursor.set((' ', 0, 0))
        cursor.apply(move)
        return

    raise BaseException("Move not implemented")


def two(cursor, move):
    return one(cursor, move)


def three(cursor, move):
    return one(cursor, move)


def four(cursor, move):
    return one(cursor, move)


def five(cursor, move):
    return one(cursor, move)


def six(cursor, move):
    return one(cursor, move)


def seven(cursor, move):
    return one(cursor, move)


def eight(cursor, move):
    return one(cursor, move)


def nine(cursor, move):
    return one(cursor, move)


flavors = [one, two, three, four, five, six, seven, eight, nine]


def pick(n):
    n = abs(n - 1)
    if n < len(flavors):
        return flavors[n]
    return default
