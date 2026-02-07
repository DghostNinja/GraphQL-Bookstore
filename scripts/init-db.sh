#!/bin/bash
# Initialize database with tables and seed data

echo "[INIT] Checking database connection..."

# Extract connection info from DATABASE_URL or DB_CONNECTION_STRING
if [ -n "$DATABASE_URL" ]; then
    export DB_CONNECTION_STRING="$DATABASE_URL"
fi

if [ -z "$DB_CONNECTION_STRING" ]; then
    echo "[INIT] No database connection string found, skipping init."
    exit 0
fi

# Parse connection string
CONN_STR="$DB_CONNECTION_STRING"
HOST=$(echo "$CONN_STR" | sed -n 's/.*@\([^:]*\):.*/\1/p')
PORT=$(echo "$CONN_STR" | sed -n 's/.*:\([0-9]*\)\/.*/\1/p')
DBNAME=$(echo "$CONN_STR" | sed -n 's/.*\/\(.*\)/\1/p' | sed 's/\?.*//')
USER=$(echo "$CONN_STR" | sed -n 's/.*:\/\([^:]*\):.*/\1/p')
export PGPASSWORD=$(echo "$CONN_STR" | sed -n 's/.*:\([^:]*\)@.*/\1/p')

echo "[INIT] Connecting to: $HOST:$PORT/$DBNAME as $USER"

# Check if users table has data
USER_COUNT=$(psql -h "$HOST" -p "$PORT" -U "$USER" -d "$DBNAME" -t -c "SELECT COUNT(*) FROM users;" 2>/dev/null | xargs)

echo "[INIT] Current user count: $USER_COUNT"

if [ "$USER_COUNT" = "0" ] || [ -z "$USER_COUNT" ]; then
    echo "[INIT] No users found, initializing database..."
    psql "$DB_CONNECTION_STRING" -f /app/scripts/init_database.sql
    echo "[INIT] Database initialized with seed data!"
else
    echo "[INIT] Database already has data ($USER_COUNT users), skipping init."
fi

echo "[INIT] Ready to start API server."
