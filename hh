cd /workspaces/MMM || exit 1

echo "🔎 البحث عن ملف ZIP..."
ls -lh *.zip

echo "📦 فك الضغط..."
mkdir -p extracted
ZIP_FILE=$(ls -t *.zip | head -1)
unzip -o "$ZIP_FILE" -d extracted

echo "🔎 البحث عن main.py داخل الملفات المفكوكة..."
MAIN_FILE=$(find extracted -type f -name "main.py" | head -1)

if [ -z "$MAIN_FILE" ]; then
  echo "❌ لم أجد main.py داخل الملف المضغوط"
  find extracted -maxdepth 3 -type f | head -50
  exit 1
fi

PROJECT_DIR=$(dirname "$MAIN_FILE")
echo "✅ مجلد المشروع الحقيقي هو: $PROJECT_DIR"

echo "🔁 نقل ملفات المشروع إلى جذر المستودع..."
rsync -av "$PROJECT_DIR"/ ./ \
  --exclude ".env" \
  --exclude ".venv" \
  --exclude "venv" \
  --exclude "__pycache__" \
  --exclude "*.pyc" \
  --exclude "*.db" \
  --exclude "backups"

echo "🛡️ تجهيز .gitignore..."
cat > .gitignore <<'EOF'
.env
*.env
__pycache__/
*.pyc
.venv/
venv/
env/
mansour_factory.db
*.db
*.db-journal
backups/
users_export.csv
downloads/
temp/
.cache/
.vscode/
.pytest_cache/
extracted/
*.zip
EOF

echo "📦 تثبيت المتطلبات..."
python -m pip install --upgrade pip

if [ -f requirements.txt ]; then
  python -m pip install -r requirements.txt
else
  python -m pip install "aiogram>=3,<4" python-dotenv aiohttp aiosqlite sqlalchemy yt-dlp
fi

echo "🧪 فحص ملفات Python..."
python -m py_compile $(find . -name "*.py" \
  -not -path "./.venv/*" \
  -not -path "./venv/*" \
  -not -path "./backups/*" \
  -not -path "./extracted/*" \
  -not -path "./__pycache__/*")

echo "✅ الفحص نجح"

echo "💾 حفظ المشروع الحقيقي في Git..."
git add .
git rm --cached "$ZIP_FILE" 2>/dev/null || true
git rm -r --cached extracted 2>/dev/null || true

git commit -m "extract telegram bot project files" || echo "ℹ️ لا توجد تغييرات جديدة"
git push -u origin "$(git branch --show-current)"

echo "▶️ تشغيل البوت..."
python main.py
