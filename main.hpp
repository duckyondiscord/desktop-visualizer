enum class BARPOSITION : int
{
    BAR_BOTTOM,
    BAR_TOP,
    BAR_MIDDLE
};

float bars[64];
int width = 800;
int height = 350;

// You can change type
BARPOSITION barposition = BARPOSITION::BAR_TOP;