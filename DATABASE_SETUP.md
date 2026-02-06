# Database Setup Instructions

## Prerequisites

PostgreSQL must be installed and running on your system.

## Setup Steps

### 1. Initialize the Database

Run the initialization script as the postgres user:

```bash
sudo -u postgres psql -f scripts/init_database.sql
```

If prompted for a password, you may need to:
1. Set a password for the postgres user, or
2. Modify pg_hba.conf to allow peer authentication

### 2. Default Users

The following users are created with password `password123`:

| Username | Password | Role |
|----------|-----------|-------|
| admin    | password123 | admin  |
| staff    | password123 | staff  |
| user     | password123 | user   |

### 3. Test Connection

Test that the database is accessible:

```bash
psql -h localhost -U postgres -d bookstore_db -c "SELECT username, role FROM users;"
```

## Alternative: No Database Mode

If you cannot set up PostgreSQL, the server will fail to start. For a demo without database:

1. Create a simple database connection wrapper
2. Or use the PostgreSQL Docker container:

```bash
docker run --name postgres-bookstore \
  -e POSTGRES_PASSWORD=password \
  -p 5432:5432 \
  -v $(pwd)/scripts/init_database.sql:/docker-entrypoint-initdb.d/init.sql \
  postgres:16
```

Then update DB_CONN in main.cpp to:
```cpp
#define DB_CONN "dbname=bookstore_db user=postgres password=password host=localhost port=5432"
```

## Troubleshooting

### Peer Authentication Error

If you get "role does not exist" or "no password supplied" errors:

Edit `/etc/postgresql/16/main/pg_hba.conf` and add:
```
local   all             postgres                                trust
host    all             all             127.0.0.1/32            trust
```

Then restart PostgreSQL:
```bash
sudo systemctl restart postgresql
```

### Connection Refused

Check if PostgreSQL is running:
```bash
sudo systemctl status postgresql
```

Start it if needed:
```bash
sudo systemctl start postgresql
```
