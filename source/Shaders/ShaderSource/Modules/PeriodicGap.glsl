/* Tests distance d (pixels from a gap's anchor corner, along its edge) against a periodic
   ###offset___length###offset___length### pattern, where '#' is solid border and '_' is gap.
   repeat selects how many gap segments to draw: 0 disables the gap entirely, 1 reproduces a
   single gap at [offset, offset+length), >1 draws that many gaps and then stops even if more
   would fit, and <0 keeps tiling for as long as the edge is - since d is derived from the live
   rectangle size every frame, this automatically grows/shrinks the dash count as it resizes. */
bool isInPeriodicGap(float d, float gapOffset, float gapLength, int repeat)
{
    if (repeat == 0 || gapLength <= 0.0 || d < 0.0)
        return false;

    float period = gapOffset + gapLength;
    if (period <= 0.0)
        return false;

    if (repeat > 0 && d >= float(repeat) * period)
        return false;

    return mod(d, period) >= gapOffset;
}

