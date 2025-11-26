#!/usr/bin/env bash
set -euo pipefail

# start.sh - 启动本地静态服务器并在默认浏览器中打开页面
# 使用方法：
#   chmod +x start.sh
#   ./start.sh [PORT]
# 默认端口 8888

PORT=${1:-8888}
LOGFILE=/tmp/vocabmemster_server.log

# 寻找前端目录候选（优先级）
CANDIDATES=(
  "$(pwd)/backend/resources/web-ui"
  "$(pwd)/frontend/dist"
  "$(pwd)/backend/build/resources/web-ui"
  "$(pwd)/resources/web-ui"
)

SERVE_DIR=""
for d in "${CANDIDATES[@]}"; do
  if [ -d "$d" ] && [ -f "$d/index.html" ]; then
    SERVE_DIR="$d"
    # 优先包含 assets 的目录
    if [ -d "$d/assets" ]; then
      break
    fi
  fi
done

if [ -z "$SERVE_DIR" ]; then
  echo "未找到可用的前端目录。尝试在仓库中构建前端：frontend -> npm run build，然后重试。"
  exit 1
fi

echo "使用前端目录: $SERVE_DIR"

# 停止可能已存在的同端口 python http.server 进程（谨慎）
pkill -f "python3 -m http.server $PORT" || true
sleep 0.2

# 启动服务器并重定向日志
cd "$SERVE_DIR"
nohup python3 -m http.server "$PORT" > "$LOGFILE" 2>&1 &
PID=$!

echo "HTTP 静态服务器已启动 (PID=$PID), 端口 $PORT, 日志: $LOGFILE"

# 等待服务器可用
URL="http://localhost:$PORT/"
TIMEOUT=10
SLEEP=0.2
ELAPSED=0
while true; do
  if curl -sSf --max-time 2 "$URL" > /dev/null 2>&1; then
    echo "服务器已就绪： $URL"
    break
  fi
  sleep $SLEEP
  ELAPSED=$(awk "BEGIN {print $ELAPSED + $SLEEP}")
  if (( $(echo "$ELAPSED >= $TIMEOUT" | bc -l) )); then
    echo "等待服务器超时（$TIMEOUT 秒），请检查 $LOGFILE"
    echo "最后 50 行日志："
    tail -n 50 "$LOGFILE" || true
    exit 1
  fi
done

# 打开默认浏览器（Linux 使用 xdg-open）
if command -v xdg-open > /dev/null 2>&1; then
  xdg-open "$URL" || true
elif [ -n "${BROWSER:-}" ]; then
  # If $BROWSER is set (devcontainer), use it
  "$BROWSER" "$URL" || true
else
  echo "请在浏览器打开: $URL"
fi

echo "要停止服务器： kill $PID 或 pkill -f 'python3 -m http.server $PORT'"

exit 0
