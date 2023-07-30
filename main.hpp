enum class BARPOSITION : int
{
    BAR_BOTTOM,
    BAR_TOP,
    BAR_MIDDLE
};

enum class WINDOW_MODE : int
{
    BACKGROUND,
    BORDER
};

float bars[64];

int width = 800;
int height = 600;

int fps = 60;

// You can change type
BARPOSITION barposition = BARPOSITION::BAR_MIDDLE;

WINDOW_MODE windowmode = WINDOW_MODE::BACKGROUND;