
CONFIG_DIRECTORY="/home/gio/CLionProjects/untitled8/config"

PROGRAM="/home/gio/CLionProjects/untitled8/build/stream_analysis"

RESULTS_DIRECTORY="/home/gio/CLionProjects/untitled8/results"

for CONFIG_FILE in "$CONFIG_DIRECTORY"/*.json; do
  BASE_NAME=$(basename "$CONFIG_FILE" .json)

  RESULTS_FILE="$RESULTS_DIRECTORY/results_$BASE_NAME.csv"
  "$PROGRAM" "$CONFIG_FILE" "$RESULTS_FILE" &
done
