#version 330 core
uniform float uOpacity = 1.0; //Shader::OpacityScope's product, uploaded with every UseProgram

uniform vec4 uColor;
uniform float uLineThickness; // the strip's full width, in pixels
uniform float uGlowWidth = 0.0; // above 0, the strip is a glow around a line uCoreThickness wide, fading out over this many pixels
uniform float uCoreThickness; // the line's own width, in pixels, which a glow surrounds
uniform int uCoverageBand = 0; // 1: only the fully covered fragments, 2: only the faded ones, 0: both

in float gs_Distance;
in vec2 gs_Position;
flat in vec4
    gs_PreviousPiece,
    gs_Piece,
    gs_NextPiece;
flat in vec2 gs_HasNeighbours;
out vec4 FragColor;

// How far point lies outside the piece of the line from piece.xy to piece.zw, halfWidth to each side (negative inside). A square
// end stops flush with its point, where the line ends; a joint between two pieces is round
float distanceToPiece(vec2 point, vec4 piece, float halfWidth, bool isStartSquare, bool isEndSquare)
{
    vec2 direction = piece.zw - piece.xy;
    float pieceLength = length(direction);

    if (pieceLength < 0.0001)
        return length(point - piece.xy) - halfWidth;

    direction /= pieceLength;
    vec2 offset = point - piece.xy;
    float
        along = dot(offset, direction),
        across = abs(dot(offset, vec2(-direction.y, direction.x)));

    if (along < 0.0 && isStartSquare)
        return length(vec2(-along, max(across - halfWidth, 0.0)));

    if (along > pieceLength && isEndSquare)
        return length(vec2(along - pieceLength, max(across - halfWidth, 0.0)));

    return length(offset - direction * clamp(along, 0.0, pieceLength)) - halfWidth;
}

// How far the fragment lies outside the line: its own piece and the ones on either side. Every segment whose glow reaches a pixel
// works out the same distance, so it doesn't matter which one draws it. The far ends of the neighbours count as round joints
float distanceToLine()
{
    float
        halfWidth = uCoreThickness * 0.5,
        distance = distanceToPiece(gs_Position, gs_Piece, halfWidth, 0.0 == gs_HasNeighbours.x, 0.0 == gs_HasNeighbours.y);

    if (0.0 < gs_HasNeighbours.x)
        distance = min(distance, distanceToPiece(gs_Position, gs_PreviousPiece, halfWidth, false, false));

    if (0.0 < gs_HasNeighbours.y)
        distance = min(distance, distanceToPiece(gs_Position, gs_NextPiece, halfWidth, false, false));

    return distance;
}

void main()
{
    // How much of the pixel the strip covers. A line: fully up to half a pixel inside its edge, not at all half a pixel outside it,
    // which Line.gs leaves room for. A glow: full up to the line's edge, then fading to nothing over its width, as the glows around
    // rectangles and text, all around the line, its square ends included
    float coverage = uGlowWidth > 0.0
        ? 1.0 - smoothstep(0.0, uGlowWidth, distanceToLine())
        : clamp(uLineThickness * 0.5 + 0.5 - abs(gs_Distance), 0.0, 1.0);

    // Nothing to draw, and it mustn't write depth either, or it would hide what is drawn behind it later
    if (coverage <= 0.0)
        discard;

    // Polyline draws the full fragments first, so where pieces of a turn overlap a pixel takes a full one over a faded edge
    bool isFull = coverage >= 1.0;

    if ((1 == uCoverageBand && !isFull) || (2 == uCoverageBand && isFull))
        discard;

    FragColor = vec4(uColor.rgb, uColor.a * coverage);
    FragColor.a *= uOpacity; //faded with the widget and whatever fades around it
}
