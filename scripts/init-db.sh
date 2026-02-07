#!/bin/bash
# Initialize database if tables don't exist

echo "[INIT] Checking database connection..."
export PGPASSWORD="${DB_PASSWORD}"
HOST=$(echo $DB_CONNECTION_STRING | sed 's/.*@\([^:]*\):.*/\1/')
PORT=$(echo $DB_CONNECTION_STRING | sed 's/.*:\([0-9]*\)\/.*/\1/')
DBNAME=$(echo $DB_CONNECTION_STRING | sed 's/.*\/\([^]]*\).*/\1/')
USER=$(echo $DB_CONNECTION_STRING | sed 's/.*:\([^:]*\)@.*/\1/')
export PGPASSWORD=$(echo $DB_CONNECTION_STRING | sed 's/.*:\([^:]*\)@.*/\1/')

# Check if users table exists
EXISTS=$(psql -h $HOST -p $PORT -U $USER -d $DBNAME -t -c "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_name = 'users');" 2>/dev/null | xargs)

if [ "$EXISTS" = "f" ]; then
    echo "[INIT] Tables don't exist, running init_database.sql..."
    psql "$DB_CONNECTION_STRING" -f /app/scripts/init_database.sql
    echo "[INIT] Database initialized!"
else
    echo "[INIT] Tables already exist, skipping init."
fi
