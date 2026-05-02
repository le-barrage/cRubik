#include "cube.h"
#include "cublet.h"
#include "include/raylib.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int SIZE = 3, MAXSIZE = 9, ROTATIONSPEED = 25;

static bool
isInnerCubie (float x, float y, float z)
{
  return x != 0 && y != 0 && z != 0 && x != SIZE - 1 && y != SIZE - 1
         && z != SIZE - 1;
}

Cube
Cube_make (float cubletSize)
{
  Cube cube;
  cube.cube = (cubie_t ***)malloc (SIZE * sizeof (cubie_t **));
  for (unsigned short int x = 0; x < SIZE; x++)
    {
      cube.cube[x] = (cubie_t **)malloc (SIZE * sizeof (cubie_t *));
      for (unsigned short int y = 0; y < SIZE; y++)
        {
          cube.cube[x][y] = (cubie_t *)malloc (SIZE * sizeof (cubie_t));
          for (unsigned short int z = 0; z < SIZE; z++)
            {
              if (isInnerCubie (x, y, z))
                continue;
              cube.cube[x][y][z] = cubie_make (x, y, z, cubletSize, SIZE);
            }
        }
    }
  cube.isAnimating = false;
  cube.rotationDegrees = 0;
  cube.currentRotation = -1;
  return cube;
}

Cube
Cube_deepCopy (Cube *src)
{
  Cube dst;
  dst.cube = (cubie_t ***)malloc (SIZE * sizeof (cubie_t **));
  for (unsigned short int x = 0; x < SIZE; x++)
    {
      dst.cube[x] = (cubie_t **)malloc (SIZE * sizeof (cubie_t *));
      for (unsigned short int y = 0; y < SIZE; y++)
        {
          dst.cube[x][y] = (cubie_t *)malloc (SIZE * sizeof (cubie_t));
          memcpy (dst.cube[x][y], src->cube[x][y], SIZE * sizeof (cubie_t));
        }
    }
  dst.isAnimating = src->isAnimating;
  dst.rotationDegrees = src->rotationDegrees;
  dst.currentRotation = src->currentRotation;
  return dst;
}

void
Cube_free (Cube cube)
{
  for (unsigned short int x = 0; x < SIZE; x++)
    {
      for (unsigned short int y = 0; y < SIZE; y++)
        {
          free (cube.cube[x][y]);
        }
      free (cube.cube[x]);
    }
  free (cube.cube);
}

static void
handleAnimating (Cube *cube)
{
  if (cube->isAnimating)
    cube->rotationDegrees += ROTATIONSPEED;
  if (cube->rotationDegrees > 90)
    {
      cube->rotationDegrees = 0;
      cube->isAnimating = false;
      Cube_rotate (cube, cube->currentRotation, 1);
      cube->currentRotation = -1;
    }
}

static Vector3
getRotationVector (Rotation rotation, int posX, int posY, int posZ)
{
  switch (rotation)
    {
    case X:
      return (Vector3){ -1, 0, 0 };
    case x:
      return (Vector3){ 1, 0, 0 };
    case Y:
      return (Vector3){ 0, -1, 0 };
    case y:
      return (Vector3){ 0, 1, 0 };
    case Z:
      return (Vector3){ 0, 0, -1 };
    case z:
      return (Vector3){ 0, 0, 1 };
    case U:
      return (posY == SIZE - 1) ? (Vector3){ 0, -1, 0 } : (Vector3){ 0, 0, 0 };
    case u:
      return (posY == SIZE - 1) ? (Vector3){ 0, 1, 0 } : (Vector3){ 0, 0, 0 };
    case D:
      return (posY == 0) ? (Vector3){ 0, 1, 0 } : (Vector3){ 0, 0, 0 };
    case d:
      return (posY == 0) ? (Vector3){ 0, -1, 0 } : (Vector3){ 0, 0, 0 };
    case E:
      return (0 < posY && posY < SIZE - 1) ? (Vector3){ 0, 1, 0 }
                                           : (Vector3){ 0, 0, 0 };
    case e:
      return (0 < posY && posY < SIZE - 1) ? (Vector3){ 0, -1, 0 }
                                           : (Vector3){ 0, 0, 0 };
    case R:
      return (posX == SIZE - 1) ? (Vector3){ -1, 0, 0 } : (Vector3){ 0, 0, 0 };
    case r:
      return (posX == SIZE - 1) ? (Vector3){ 1, 0, 0 } : (Vector3){ 0, 0, 0 };
    case L:
      return (posX == 0) ? (Vector3){ 1, 0, 0 } : (Vector3){ 0, 0, 0 };
    case l:
      return (posX == 0) ? (Vector3){ -1, 0, 0 } : (Vector3){ 0, 0, 0 };
    case M:
      return (0 < posX && posX < SIZE - 1) ? (Vector3){ 1, 0, 0 }
                                           : (Vector3){ 0, 0, 0 };
    case m:
      return (0 < posX && posX < SIZE - 1) ? (Vector3){ -1, 0, 0 }
                                           : (Vector3){ 0, 0, 0 };
    case F:
      return (posZ == SIZE - 1) ? (Vector3){ 0, 0, -1 } : (Vector3){ 0, 0, 0 };
    case f:
      return (posZ == SIZE - 1) ? (Vector3){ 0, 0, 1 } : (Vector3){ 0, 0, 0 };
    case B:
      return (posZ == 0) ? (Vector3){ 0, 0, 1 } : (Vector3){ 0, 0, 0 };
    case b:
      return (posZ == 0) ? (Vector3){ 0, 0, -1 } : (Vector3){ 0, 0, 0 };
    case S:
      return (0 < posZ && posZ < SIZE - 1) ? (Vector3){ 0, 0, -1 }
                                           : (Vector3){ 0, 0, 0 };
    case s:
      return (0 < posZ && posZ < SIZE - 1) ? (Vector3){ 0, 0, 1 }
                                           : (Vector3){ 0, 0, 0 };
    default:
      return (Vector3){ 0, 0, 0 };
    }
}

static void
handleAnimation (Cube *cube, int posX, int posY, int posZ)
{
  Vector3 position = (Vector3){ posX - (float)SIZE / 2 + 0.5f,
                                posY - (float)SIZE / 2 + 0.5f,
                                posZ - (float)SIZE / 2 + 0.5f };

  Vector3 direction
      = getRotationVector (cube->currentRotation, posX, posY, posZ);
  int rotationDegrees
      = (direction.x == 0 && direction.y == 0 && direction.z == 0)
            ? 0
            : cube->rotationDegrees;
  cubie_draw (&cube->cube[posX][posY][posZ], position, direction,
                   rotationDegrees);
}

void
Cube_drawCube (Cube *cube)
{
  handleAnimating (cube);
  for (unsigned short int x = 0; x < SIZE; x++)
    for (unsigned short int y = 0; y < SIZE; y++)
      for (unsigned short int z = 0; z < SIZE; z++)
        {
          if (isInnerCubie (x, y, z))
            continue;
          handleAnimation (cube, x, y, z);
        }
}

Rotation
getCorrespondingRotation (char c)
{
  switch (c)
    {
    case 'U':
      return U;
    case 'u':
      return u;
    case 'D':
      return D;
    case 'd':
      return d;
    case 'R':
      return R;
    case 'r':
      return r;
    case 'L':
      return L;
    case 'l':
      return l;
    case 'F':
      return F;
    case 'f':
      return f;
    case 'B':
      return B;
    case 'b':
      return b;
    case 'M':
      return M;
    case 'm':
      return m;
    case 'E':
      return E;
    case 'e':
      return e;
    case 'S':
      return S;
    case 's':
      return s;
    case 'X':
      return X;
    case 'x':
      return x;
    case 'Y':
      return Y;
    case 'y':
      return y;
    case 'Z':
      return Z;
    case 'z':
      return z;
    default:
      return -1;
    }
}

static char
getRotation (const char *move, size_t len)
{
  if (move[len - 1] == '\'')
    return tolower (move[len - 2]);
  else if (move[len - 1] == '2')
    return move[len - 2];
  else
    return move[len - 1];
}

void
Cube_applyMove (Cube *cube, char *move)
{
  size_t len = strlen (move);
  int nbOfLayers;
  if (move[1] == 'w')
    nbOfLayers = move[0] - '0';
  else
    nbOfLayers = (move[0] - '0') * 10 + move[1] - '0';
  char rotation = getRotation (move, len);
  if (move[len - 1] == '2')
    Cube_rotate (cube, getCorrespondingRotation (rotation), nbOfLayers);
  Cube_rotate (cube, getCorrespondingRotation (rotation), nbOfLayers);
}

/*----------------------------------------------------------------*/

static unsigned short int
calculateX (int dirX, int i)
{
  return (dirX == -1) ? i : dirX;
}

static unsigned short int
calculateY (int dirX, int dirY, int i, int j)
{
  return (dirY == -1) ? (dirX == -1) ? j : i : dirY;
}

static unsigned short int
calculateZ (int dirZ, int j)
{
  return (dirZ == -1) ? j : dirZ;
}

static void
storeFaceAndRotateCubies (Cube *cube, Vector3 dir,
                          void (*cubieRotation) (cubie_t *),
                          cubie_t face[SIZE][SIZE])
{
  unsigned short int x, y, z;
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      {
        x = calculateX (dir.x, i);
        y = calculateY (dir.x, dir.y, i, j);
        z = calculateZ (dir.z, j);
        cubieRotation (&cube->cube[x][y][z]);
        face[i][j] = cube->cube[x][y][z];
      }
}

static void
transposeMatrix (cubie_t face[SIZE][SIZE])
{
  for (int i = 0; i < SIZE; i++)
    for (int j = i + 1; j < SIZE; j++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[j][i];
        face[j][i] = temp;
      }
}

static void
reverseRows (cubie_t face[SIZE][SIZE])
{
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE / 2; j++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[i][SIZE - j - 1];
        face[i][SIZE - j - 1] = temp;
      }
}

static void
reverseColumns (cubie_t face[SIZE][SIZE])
{
  for (int j = 0; j < SIZE; j++)
    for (int i = 0; i < SIZE / 2; i++)
      {
        cubie_t temp = face[i][j];
        face[i][j] = face[SIZE - i - 1][j];
        face[SIZE - i - 1][j] = temp;
      }
}

static void
updateCubeFace (Cube *cube, Vector3 dir, cubie_t face[SIZE][SIZE])
{
  unsigned short int x, y, z;
  for (int i = 0; i < SIZE; i++)
    for (int j = 0; j < SIZE; j++)
      {
        x = calculateX (dir.x, i);
        y = calculateY (dir.x, dir.y, i, j);
        z = calculateZ (dir.z, j);
        cube->cube[x][y][z] = face[i][j];
      }
}

static void
rotate (Cube *cube, Vector3 dir, void (*cubieRotation) (cubie_t *),
        bool antiClockwise)
{
  cubie_t face[SIZE][SIZE];
  storeFaceAndRotateCubies (cube, dir, cubieRotation, face);
  transposeMatrix (face);
  if (antiClockwise)
    reverseRows (face);
  else
    reverseColumns (face);
  updateCubeFace (cube, dir, face);
}

void
Cube_rotate (Cube *cube, Rotation rotation, int numberOfLayers)
{
  switch (rotation)
    {
    case U:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, SIZE - i - 1, -1 }, cubie_rotate_left,
                  false);
        break;
      }
    case u:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, SIZE - i - 1, -1 }, cubie_rotate_right,
                  true);
        break;
      }
    case D:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, i, -1 }, cubie_rotate_right, true);
        break;
      }
    case d:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, i, -1 }, cubie_rotate_left, false);
        break;
      }
    case R:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ SIZE - i - 1, -1, -1 }, cubie_rotate_up,
                  true);
        break;
      }
    case r:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ SIZE - i - 1, -1, -1 }, cubie_rotate_down,
                  false);
        break;
      }
    case L:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ i, -1, -1 }, cubie_rotate_down, false);
        break;
      }
    case l:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ i, -1, -1 }, cubie_rotate_up, true);
        break;
      }
    case F:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, SIZE - i - 1 },
                  cubie_rotate_clockwise, true);
        break;
      }
    case f:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, SIZE - i - 1 },
                  cubie_rotate_anticlockwise, false);
        break;
      }
    case B:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, i }, cubie_rotate_anticlockwise,
                  false);
        break;
      }
    case b:
      {
        for (int i = 0; i < numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, i }, cubie_rotate_clockwise, true);
        break;
      }
    case M:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ i, -1, -1 }, cubie_rotate_down, false);
        break;
      }
    case m:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ i, -1, -1 }, cubie_rotate_up, true);
        break;
      }
    case E:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, i, -1 }, cubie_rotate_right, true);
        break;
      }
    case e:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, i, -1 }, cubie_rotate_left, false);
        break;
      }
    case S:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, i }, cubie_rotate_clockwise, true);
        break;
      }
    case s:
      {
        for (int i = numberOfLayers; i < SIZE - numberOfLayers; i++)
          rotate (cube, (Vector3){ -1, -1, i }, cubie_rotate_anticlockwise,
                  false);
        break;
      }
    case X:
      {
        Cube_rotate (cube, R, SIZE / 2);
        Cube_rotate (cube, m, SIZE / 2);
        Cube_rotate (cube, l, SIZE / 2);
        break;
      }
    case x:
      {
        Cube_rotate (cube, r, SIZE / 2);
        Cube_rotate (cube, M, SIZE / 2);
        Cube_rotate (cube, L, SIZE / 2);
        break;
      }
    case Y:
      {
        Cube_rotate (cube, U, SIZE / 2);
        Cube_rotate (cube, e, SIZE / 2);
        Cube_rotate (cube, d, SIZE / 2);
        break;
      }
    case y:
      {
        Cube_rotate (cube, u, SIZE / 2);
        Cube_rotate (cube, E, SIZE / 2);
        Cube_rotate (cube, D, SIZE / 2);
        break;
      }
    case Z:
      {
        Cube_rotate (cube, F, SIZE / 2);
        Cube_rotate (cube, S, SIZE / 2);
        Cube_rotate (cube, b, SIZE / 2);
        break;
      }
    case z:
      {
        Cube_rotate (cube, f, SIZE / 2);
        Cube_rotate (cube, s, SIZE / 2);
        Cube_rotate (cube, B, SIZE / 2);
        break;
      }
    }
}

char
Cube_getFaceFromColor (cubie_t cubie, face_t face)
{
  Color color = cubie.colors[face];

  if (colors_equal (color, WHITE))
    return 'U';
  else if (colors_equal (color, GREEN))
    return 'F';
  else if (colors_equal (color, RED))
    return 'R';
  else if (colors_equal (color, BLUE))
    return 'B';
  else if (colors_equal (color, ORANGE))
    return 'L';
  else if (colors_equal (color, YELLOW))
    return 'D';
  else
    return '?';
}

char *
Cube_toString (Cube *cube, char cubeStr[55])
{
  int index = 0;
  for (int z = 0; z < SIZE; z++)
    for (int x = 0; x < SIZE; x++)
      {
        cubeStr[index]
            = Cube_getFaceFromColor (cube->cube[x][SIZE - 1][z], FACE_UP);
        index++;
      }
  for (int x = SIZE - 1; x >= 0; x--)
    for (int z = SIZE - 1; z >= 0; z--)
      {
        cubeStr[index]
            = Cube_getFaceFromColor (cube->cube[SIZE - 1][x][z], FACE_RIGHT);
        index++;
      }
  for (int z = SIZE - 1; z >= 0; z--)
    for (int x = 0; x < SIZE; x++)
      {
        cubeStr[index]
            = Cube_getFaceFromColor (cube->cube[x][z][SIZE - 1], FACE_FRONT);
        index++;
      }
  for (int z = SIZE - 1; z >= 0; z--)
    for (int x = 0; x < SIZE; x++)
      {
        cubeStr[index] = Cube_getFaceFromColor (cube->cube[x][0][z], FACE_DOWN);
        index++;
      }
  for (int x = SIZE - 1; x >= 0; x--)
    for (int z = 0; z < SIZE; z++)
      {
        cubeStr[index] = Cube_getFaceFromColor (cube->cube[0][x][z], FACE_LEFT);
        index++;
      }
  for (int z = SIZE - 1; z >= 0; z--)
    for (int x = SIZE - 1; x >= 0; x--)
      {
        cubeStr[index] = Cube_getFaceFromColor (cube->cube[x][z][0], FACE_BACK);
        index++;
      }
  cubeStr[index] = '\0';
  return cubeStr;
}

/*----------------------------------------------------------------*/

static const Color CANONICAL_COLOR[6] = {
  [FACE_UP] = WHITE,   [FACE_FRONT] = GREEN, [FACE_RIGHT] = RED,
  [FACE_BACK] = BLUE,  [FACE_LEFT] = ORANGE, [FACE_DOWN] = YELLOW,
};

static Color
worldCenterColor (Cube *cube, face_t face)
{
  int mid = SIZE / 2;
  switch (face)
    {
    case FACE_UP:    return cube->cube[mid][SIZE - 1][mid].colors[FACE_UP];
    case FACE_DOWN:  return cube->cube[mid][0][mid].colors[FACE_DOWN];
    case FACE_FRONT: return cube->cube[mid][mid][SIZE - 1].colors[FACE_FRONT];
    case FACE_BACK:  return cube->cube[mid][mid][0].colors[FACE_BACK];
    case FACE_LEFT:  return cube->cube[0][mid][mid].colors[FACE_LEFT];
    case FACE_RIGHT: return cube->cube[SIZE - 1][mid][mid].colors[FACE_RIGHT];
    }
  return BLACK;
}

static face_t
findWorldFaceShowing (Cube *cube, Color target)
{
  face_t all[6] = { FACE_UP, FACE_FRONT, FACE_RIGHT, FACE_BACK, FACE_LEFT, FACE_DOWN };
  for (int i = 0; i < 6; i++)
    if (colors_equal (worldCenterColor (cube, all[i]), target))
      return all[i];
  return FACE_UP;
}

CubeOrientation
Cube_detectOrientationAndNormalize (Cube *src, Cube *outCanonical)
{
  CubeOrientation o = { .count = 0 };
  for (int f = 0; f < 6; f++)
    o.faceMap[f] = findWorldFaceShowing (src, CANONICAL_COLOR[f]);

  *outCanonical = Cube_deepCopy (src);

  face_t whitePos = findWorldFaceShowing (outCanonical, WHITE);
  switch (whitePos)
    {
    case FACE_UP:
      break;
    case FACE_FRONT:
      Cube_rotate (outCanonical, X, SIZE / 2);
      o.moves[o.count++] = X;
      break;
    case FACE_BACK:
      Cube_rotate (outCanonical, x, SIZE / 2);
      o.moves[o.count++] = x;
      break;
    case FACE_LEFT:
      Cube_rotate (outCanonical, Z, SIZE / 2);
      o.moves[o.count++] = Z;
      break;
    case FACE_RIGHT:
      Cube_rotate (outCanonical, z, SIZE / 2);
      o.moves[o.count++] = z;
      break;
    case FACE_DOWN:
      Cube_rotate (outCanonical, X, SIZE / 2);
      Cube_rotate (outCanonical, X, SIZE / 2);
      o.moves[o.count++] = X;
      o.moves[o.count++] = X;
      break;
    }

  face_t greenPos = findWorldFaceShowing (outCanonical, GREEN);
  switch (greenPos)
    {
    case FACE_FRONT:
      break;
    case FACE_RIGHT:
      Cube_rotate (outCanonical, Y, SIZE / 2);
      o.moves[o.count++] = Y;
      break;
    case FACE_LEFT:
      Cube_rotate (outCanonical, y, SIZE / 2);
      o.moves[o.count++] = y;
      break;
    case FACE_BACK:
      Cube_rotate (outCanonical, Y, SIZE / 2);
      Cube_rotate (outCanonical, Y, SIZE / 2);
      o.moves[o.count++] = Y;
      o.moves[o.count++] = Y;
      break;
    default:
      break;
    }

  return o;
}

static char
rotationLetter (Rotation r)
{
  switch (r)
    {
    case X: case x: return 'X';
    case Y: case y: return 'Y';
    case Z: case z: return 'Z';
    default:        return '?';
    }
}

static bool
rotationIsInverse (Rotation r)
{
  return r == x || r == y || r == z;
}

void
Cube_appendNormalizationTokens (char *buf, const CubeOrientation *o)
{
  int len = strlen (buf);
  for (int i = 0; i < o->count;)
    {
      Rotation r = o->moves[i];
      char letter = rotationLetter (r);
      if (i + 1 < o->count && o->moves[i + 1] == r)
        {
          buf[len++] = letter;
          buf[len++] = '2';
          buf[len++] = ' ';
          i += 2;
        }
      else
        {
          buf[len++] = letter;
          if (rotationIsInverse (r))
            buf[len++] = '\'';
          buf[len++] = ' ';
          i++;
        }
    }
  buf[len] = '\0';
}

char
Cube_faceLetter (face_t f)
{
  switch (f)
    {
    case FACE_UP:    return 'U';
    case FACE_RIGHT: return 'R';
    case FACE_FRONT: return 'F';
    case FACE_DOWN:  return 'D';
    case FACE_LEFT:  return 'L';
    case FACE_BACK:  return 'B';
    }
  return '?';
}

/* Rotation enum is laid out as 12 (uppercase, lowercase) pairs:
 * U/u, D/d, R/r, L/l, F/f, B/b, M/m, E/e, S/s, X/x, Y/y, Z/z. The
 * lowercase form is the inverse, rendered as a primed token. 
*/
void
Cube_rotationToken (Rotation r, char out[3])
{
  static const char letters[12]
      = { 'U', 'D', 'R', 'L', 'F', 'B', 'M', 'E', 'S', 'X', 'Y', 'Z' };
  out[0] = letters[r / 2];
  if (r % 2 == 1)
    {
      out[1] = '\'';
      out[2] = '\0';
    }
  else
    out[1] = '\0';
}
