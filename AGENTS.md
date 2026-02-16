# AGENTS.md - Codebase Guide for AI Agents

## CRITICAL RULES

### NEVER use git commands without explicit permission
- NEVER run `git checkout`, `git restore`, `git reset`, or any git operations that modify the codebase
- If something is broken, ASK the user how they want to proceed
- Do not assume you can revert changes - the user may have local work they haven't pushed yet
- Always ask before making any git operations

## Quick Start

```bash
# Clone and setup (installs deps, builds, sets up database)
git clone <repo-url>
cd GraphQL-Bookstore
./build.sh

# Run the server
./bookstore-server
```

Server runs on http://localhost:4000/

## Default Credentials
| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

## Manual Build (if needed)

```bash
# Build only
g++ -std=c++17 -o bookstore-server src/main.cpp -lpq -ljwt -lcurl -lssl -lcrypto

# Or use build script
./build.sh
```

## Testing

```bash
./test_api.sh
```

### Testing Commands

```bash
# Run full test suite (requires server running)
./test_api.sh

# Test with custom API URL
API_URL=http://localhost:4000/graphql ./test_api.sh

# Test Docker container
chmod +x test_api.sh
API_URL=http://localhost:4001/graphql ./test_api.sh
```

### Test File Format (Avoid Bash Escaping)
Always use files for JSON payloads to avoid bash escaping issues:

```bash
# Create test file
cat > /tmp/test_login.json << 'EOF'
{"query":"mutation { login(username: \"admin\", password: \"password123\") { success token } }"}
EOF

# Use file input
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/test_login.json
```

### Test Results
The test script checks:
1. ✓ Server health / GraphQL Playground
2. ✓ User registration
3. ✓ User login
4. ✓ Books query (no auth)
5. ✓ 'me' query (with auth)
6. ✓ SSRF endpoint
7. ✓ BOLA vulnerability (intentional)
8. ✓ SQL Injection endpoint
9. ✓ Admin endpoints (intentional - no auth)
10. ✓ Complex nested queries
11. ✓ GraphQL introspection
12. ✓ Mass Assignment vulnerability
13. ✓ IDOR vulnerabilities
14. ✓ CORS headers

**Exit Codes:**
- `0` = All tests passed
- `1` = Some tests failed

### Clean Up
```bash
# Kill server on port 4000
pkill -f bookstore-server

# Or kill port 4000
lsof -ti:4000 | xargs kill -9
```

### Docker Deployment
```bash
# Build and run with docker-compose
docker-compose up --build

# Server will be available at http://localhost:4000
# Database runs automatically in container

# Stop containers
docker-compose down

# Rebuild after code changes
docker-compose up --build --force-recreate
```

### Environment Variables
The following environment variables can be set to configure the server:
- `PORT` - Server port (default: 4000)
- `JWT_SECRET` - JWT signing secret (default: hardcoded weak secret)
- `DB_CONNECTION_STRING` - PostgreSQL connection string
- `DATABASE_URL` - PostgreSQL connection URL (preferred)

Example:
```bash
export PORT=4000
export JWT_SECRET="your-secret-here"
export DATABASE_URL="postgresql://user:pass@host:5432/dbname?sslmode=require"
./bookstore-server
```

### GitHub Actions
The workflow (`.github/workflows/fly-deploy.yml`) runs:
1. Build Docker image
2. Test with Docker Compose (local PostgreSQL)
3. Push to Docker Hub
4. Deploy to Fly.io

Required GitHub Secrets:
- `DOCKER_USERNAME` - Docker Hub username
- `DOCKER_PASSWORD` - Docker Hub password/access token
- `FLY_API_TOKEN` - Fly.io API token (get with `flyctl auth token`)

## Code Style Guidelines

### Language & Standards
- Language: C++17
- Compiler: g++
- No external web frameworks (raw socket HTTP server)

### Naming Conventions
- **Defines**: `UPPER_CASE_WITH_UNDERSCORES` (e.g., `PORT`, `BUFFER_SIZE`, `JWT_SECRET`)
- **Structs**: `PascalCase` (e.g., `User`, `Book`, `Order`)
- **Struct Members**: `camelCase` (e.g., `username`, `passwordHash`, `firstName`)
- **Functions**: `camelCase` (e.g., `loadUsersCache`, `handleQuery`, `escapeJson`)
- **Variables**: `camelCase` (e.g., `cartId`, `targetOrderId`, `paramValues`)
- **Global Variables**: `camelCase` with descriptive names (e.g., `usersCache`, `booksCache`)

### Import/Include Order
System headers first, then third-party libs:
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <postgresql/libpq-fe.h>
#include <jwt.h>
#include <curl/curl.h>
```

### Type Usage
- Use `string` for text fields
- Use `int` for numeric IDs (books, reviews, cart items)
- Use `string` for UUID IDs (users, orders, webhooks)
- Use `double` for prices/monetary values
- Use `bool` for flags
- Use `map<K, V>` for caches keyed by ID
- Use `vector<T>` for lists

### Function Return Types
- `string` for JSON responses
- `bool` for success/failure operations
- `T*` for pointer returns (e.g., `User* getUserByUsername()`)
- `void` for procedures with no return

### Error Handling
- Database operations: Check `PQresultStatus(res)` against `PGRES_TUPLES_OK` or `PGRES_COMMAND_OK`
- JWT operations: Check return codes, return empty objects on failure
- CURL operations: Check `CURLcode res == CURLE_OK`
- Always `PQclear(res)` after use
- Return meaningful error messages in JSON responses

### JSON Building Pattern
Use `stringstream` for building responses:
```cpp
stringstream ss;
ss << "{";
ss << "\"id\":\"" << id << "\",";
ss << "\"username\":\"" << escapeJson(username) << "\"";
ss << "}";
return ss.str();
```

### Database Query Pattern
Use parameterized queries (when not intentionally vulnerable):
```cpp
string sql = "SELECT id, name FROM table WHERE id = $1";
const char* paramValues[1] = {value.c_str()};
PGresult* res = PQexecParams(dbConn, sql.c_str(), 1, nullptr, paramValues, nullptr, nullptr, 0);
// Process result
PQclear(res);
```

### Debug Output
- Use `cerr << "[DEBUG] message"` for debug logging
- Debug messages should help track flow and identify issues
- Example: `cerr << "[DEBUG] Insert successful" << endl;`

### Project Structure
```
src/main.cpp           # Main server
scripts/init_database.sql  # Database schema and seed data
build.sh              # Build script
test_api.sh           # Integration tests
docker-compose.yml     # Docker deployment
Dockerfile            # Container image
fly.toml              # Fly.io config
deploy-fly.sh         # Fly.io deployment script
```

### Vulnerability Implementation Notes
- Vulnerabilities are INTENTIONAL and realistic
- Follow existing patterns for new vulnerabilities
- No "backdoors" - use subtle implementation errors
- Use `{"name": "_internalName"}` pattern for internal/debug endpoints
- Document vulnerabilities in VULNERABILITIES.md

### Important Constants
```cpp
PORT 4000
JWT_SECRET "CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024"
DB_CONN "dbname=bookstore_db user=bookstore_user password=bookstore_password host=localhost port=5432"
```

### Default Credentials
| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

### GraphQL Request Handling
1. Parse Authorization header to extract JWT token
2. Verify JWT with `verifyJWT()`, get User object
3. Parse query string from POST body
4. Check for "mutation" or "query" keyword
5. Route to `handleMutation()` or `handleQuery()`
6. Return JSON response wrapped in `{"data":{...}}`

### Adding New Queries/Mutations
1. Add struct for new data type if needed
2. Add cache variable: `map<K, T> newCache;`
3. Add `loadNewCache()` function
4. Call `loadNewCache()` in `main()` after db connect
5. Add handler in `handleQuery()` or `handleMutation()`
6. Add introspection entry in `__schema` query handler (NO descriptions!)
7. Add JSON builder function if needed: `TToJson(const T& t)`

### Comment Policy
- DO NOT add code comments
- Keep code concise and self-explanatory
- Comments in documentation only (README, VULNERABILITIES.md)

### Available Queries
| Query | Description | Auth Required |
|-------|-------------|---------------|
| `me` | Get current authenticated user | Yes |
| `books` | List all books with optional search and category filter | No |
| `book(id)` | Get a specific book by ID | No |
| `cart` | Get user's shopping cart | Yes |
| `orders` | Get user's orders | Yes |
| `bookReviews(bookId)` | Get reviews for a specific book | No |
| `myReviews` | Get current user's reviews | Yes |
| `webhooks` | Get user's registered webhooks | Yes |
| `_internalUserSearch(username)` | Internal user search | No |
| `_fetchExternalResource(url)` | Fetch external resource by URL | No |
| `_searchAdvanced(query)` | Advanced search | No |
| `_adminStats` | Admin statistics | No |
| `_adminAllOrders` | All orders | No |
| `_adminAllPayments` | All payment transactions | No |
| `_proInventory` | Pro-level books collection | No |

### Available Mutations
| Mutation | Description | Auth Required |
|----------|-------------|---------------|
| `register(username, firstName, lastName, password)` | Register a new user | No |
| `login(username, password)` | Login and get JWT token | No |
| `updateProfile(...)` | Update user profile | Yes |
| `addToCart(bookId, quantity)` | Add item to shopping cart | Yes |
| `removeFromCart(bookId)` | Remove item from shopping cart | Yes |
| `createOrder()` | Create order from cart | Yes |
| `cancelOrder(orderId)` | Cancel an order | Yes |
| `createReview(bookId, rating, comment)` | Create a review | Yes |
| `deleteReview(reviewId)` | Delete a review | Yes |
| `registerWebhook(url, events, secret)` | Register a webhook URL | Yes |
| `testWebhook(webhookId)` | Test a webhook | Yes |

### Recent Features Added
- **Field Selection**: All queries now return only requested fields (e.g., `{ books { id title } }` returns only id and title)
- **JWT Enhancements**: Tokens now include `iat` (issued at) and `exp` (expires in 6 hours)
- **Nested Author Field**: Books can include author details via `author { firstName lastName }`
- **Authors Cache**: Authors are loaded at startup for nested queries
- **Shopping Cart System**: Full cart functionality with add/remove items
- **Order Management**: Create orders from cart, cancel orders
- **Review System**: Create and delete reviews
- **Webhook System**: Register webhooks with SSRF via testWebhook
- **Admin Queries**: Stats, orders, and payments accessible without auth
- **Rate Limiting**: IP-based rate limiting (100 requests/minute, 5-min block)
- **API Documentation Page**: Beautiful glass-morphism landing page with query runner
- **Hidden Pro Inventory**: `_proInventory` query reveals 6 hidden expert-level books
- **Hidden Endpoints**: Additional hidden API endpoints for advanced security testing
- **Pro Vulnerabilities**: 6 additional expert-level security vulnerabilities

### Hidden Pro Vulnerabilities
The server contains 6 hidden expert-level vulnerabilities:

| Query | Description |
|-------|-------------|
| `_batchQuery` | GraphQL batch queries bypass rate limiting |
| `_processXML` | XXE vulnerability in XML processing |
| `_applyCouponRace` | Race condition in coupon application |
| `_jwtAlgorithmConfusion` | JWT algorithm confusion attack |
| `_cachePoison` | HTTP cache poisoning via headers |
| `_deepRecursion` | Deep recursion attack via nested queries |

### Hidden Pro Books (via `_proInventory`)
The server contains 6 hidden books with advanced security research content:

| Book Key | Title | Difficulty | Hint |
|----------|-------|------------|------|
| quantum_cryptography | Quantum Cryptography: The Next Frontier | master | Look for patterns in the API rate limiting headers |
| zero_day_exploits | Zero-Day Exploits: Offensive Security | master | Check for timing attacks in authentication |
| ai_red_team | AI Red Teaming: Advanced Adversarial ML | master | GraphQL batch queries bypass rate limits |
| blockchain_hacking | Blockchain Hacking: DeFi Vulnerabilities | master | Look for second-order vulnerabilities |
| memory_forensics | Advanced Memory Forensics | master | Check XML parsing for XXE |
| apt_analysis | APT Analysis: Nation-State Threats | master | WebSocket connections may leak data |

### Hidden API Endpoints
These endpoints provide advanced API functionality:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/webhooks/subscribe` | POST | Subscribe to webhook events |
| `/api/batch` | POST | Batch GraphQL queries |
| `/api/coupons/apply` | POST | Apply coupon codes |
| `/api/debug/timing` | GET | Debug timing information |

### Advanced Features
The server contains additional advanced features for expert-level testing:

1. **Timing Analysis**: Login response times differ based on username validity (25ms vs 50ms delay)
2. **Second-Order Effects**: Review comments are logged and queried for analytics
3. **XML Processing**: XML webhook subscription endpoint handles XML payloads
4. **Batch Queries**: GraphQL batch queries for efficient data loading
5. **Concurrent Operations**: Coupon application handles concurrent requests
6. **Debug Endpoints**: Timing information available for debugging

### Rate Limiting
The server includes built-in rate limiting to prevent abuse:
- **100 requests per 60 seconds** per IP address
- **5-minute block** when limit exceeded
- **Automatic cleanup** of old entries every 60 seconds

Rate limit configuration constants in `src/main.cpp`:
```cpp
#define RATE_LIMIT_WINDOW_SECONDS 60
#define RATE_LIMIT_MAX_REQUESTS 100
#define RATE_LIMIT_BLOCK_DURATION 300
```

### API Documentation Page
The landing page (`generateLandingHTML()` in `src/main.cpp`) provides:
- Glass-morphism UI design with animated backgrounds
- **API Link Bar**: Small glass icon with pulse animation and copyable link to `api.graphqlbook.store/graphql`
- Query Runner panel for testing GraphQL queries
- Login and Registration panels with JWT token storage
- Quick examples and available endpoints grid
- Click-to-load endpoint examples
- Vulnerability chapter slideshow

**NOTE**: No built-in GraphQL Playground. Use external tools like:
- https://studio.apollographql.com/
- Postman, Insomnia, curl, Burp Suite

**API Link Bar Features:**
- Glass-styled icon with green gradient and pulse animation effect
- Clickable API URL that copies to clipboard when clicked
- Shows "Copied!" feedback for 2 seconds after clicking
- Styled with italic serif font for "Access the API at:" label

**Query Runner Features:**
- Textarea for entering GraphQL queries
- "Load Sample" button loads books query example
- "Run Query" button executes query via XMLHttpRequest
- Response displayed with JSON formatting
- Optional username/password for authenticated requests

**Authentication Panel Features:**
- Separate Login and Register panels
- JWT tokens stored in localStorage
- Tokens automatically used for authenticated requests

### Fly.io Deployment
App name: `graphql-bookstore`
URL: https://graphql-bookstore.fly.dev

### Testing New Features
```bash
# Test cart with authentication
cat > /tmp/test_login.json << 'EOF'
{"query":"mutation { login(username: \"admin\", password: \"password123\") { token } }"}
EOF
TOKEN=$(curl -s -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/test_login.json | grep -oP '"token":"[^"]+' | cut -d'"' -f4)

cat > /tmp/test_cart.json << 'EOF'
{"query":"mutation { addToCart(bookId: 1, quantity: 2) { success message } }"}
EOF
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  --data-binary @/tmp/test_cart.json

# Test order creation
cat > /tmp/test_order.json << 'EOF'
{"query":"mutation { createOrder { success orderId totalAmount } }"}
EOF
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  --data-binary @/tmp/test_order.json

# Test admin queries (no auth required!)
cat > /tmp/test_admin.json << 'EOF'
{"query":"query { _adminStats { userCount bookCount totalRevenue } }"}
EOF
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/test_admin.json

# Test SQL injection
cat > /tmp/test_sql.json << 'EOF'
{"query":"query { _searchAdvanced(query: \"1 OR 1=1\") { id title } }"}
EOF
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/test_sql.json
```

### SSRF URL Whitelist
Allowed prefixes for `_fetchExternalResource`:
- `http://example.com`
- `http://httpbin.org`
- `http://api.github.com`, `https://api.github.com`
- `http://169.254.169.254` (cloud metadata)
- `http://localhost:`, `http://127.0.0.1:`

---

## CRITICAL: GraphQL Query Parsing Guidelines

### The Backslash-Escaped Quote Problem

When bash receives curl commands with `\"` inside single-quoted JSON, bash adds additional backslashes. For example:

```bash
# What you TYPE:
-d '{"query":"mutation { login(username: \"admin\", password: \"password123\") { success } }"}'

# What the SERVER receives:
{"query":"mutation { login(username: \\"admin\\", password: \\"password123\\") { success } }"}
```

The `\"` becomes `\\"` - the backslash is LITERAL in the string.

### Correct extractValue() Implementation

This is the CORRECT implementation that handles escaped quotes:

```cpp
string extractValue(const string& query, const string& key) {
    string searchKey = key + ":";
    size_t keyPos = query.find(searchKey);
    if (keyPos == string::npos) return "";
    
    size_t searchStart = keyPos + searchKey.length();
    
    // Skip whitespace
    while (searchStart < query.length() && 
           (query[searchStart] == ' ' || query[searchStart] == '\t')) {
        searchStart++;
    }
    
    if (searchStart >= query.length()) return "";
    
    // Skip opening quote (may be escaped with backslash like \")
    if (query[searchStart] == '"') {
        searchStart++;
    } else if (query[searchStart] == '\\' && 
               searchStart + 1 < query.length() && 
               query[searchStart + 1] == '"') {
        // Skip escaped quote: \"
        searchStart += 2;
    }
    
    string value;
    bool escaped = false;
    
    for (size_t i = searchStart; i < query.length(); i++) {
        char c = query[i];
        
        if (escaped) {
            // If we're escaped and see a quote, it's an escaped quote - skip it
            if (c != '"') {
                value += c;
            }
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            // End of string
            return value;
        } else if (c == ' ' || c == ',' || c == ')' || c == '{' || c == '}') {
            // End of value (unquoted)
            return value;
        } else {
            value += c;
        }
    }
    
    return value;
}
```

### Key Points:
1. Check for both `"` (unescaped) AND `\"` (escaped) as opening quotes
2. When `escaped=true`, a `"` means an escaped quote - skip it, don't add to value
3. Only return when you hit an UNESCAPED closing quote
4. Handle whitespace, commas, parentheses, and braces as value delimiters

### Testing Query Parsing

ALWAYS test with debug logging enabled:

```cpp
cerr << "[DEBUG] Raw body: " << body << endl;
cerr << "[DEBUG] Extracted query: " << queryStr << endl;
cerr << "[DEBUG] username='" << username << "', password='" << password << "'" << endl;
```

If you see `username='"admin"'` (with quotes included), the parsing is broken.

### Never Use Regex for This

Regex is fragile with escaped strings. Use the simple character-by-character parsing shown above.

### If You Break This Again...

Symptoms to watch for:
- `{"data":{"login":{"success":false,"message":"Missing required fields: username, password"}}}`
- Logs show `username='"admin"'` or `username=''`
- Server receives `\\"` in the raw body

Fix: Use the extractValue() implementation above.

---

## Fly.io Deployment

### Why Fly.io?
- Faster cold starts than Render
- Better performance for low-traffic apps
- Generous free tier
- Docker-based deployments
- Automatic HTTPS
- Edge caching

### Initial Setup

```bash
# Install flyctl
curl -L https://fly.io/install.sh | sh
export FLYCTL_INSTALL="$HOME/.fly"
export PATH="$FLYCTL_INSTALL/bin:$PATH"

# Authenticate
flyctl auth login

# Launch app (creates fly.toml and deploys)
./deploy-fly.sh
```

### Manual Deployment
```bash
# Deploy to Fly.io
fly deploy

# Check status
fly status

# View logs
fly logs

# Open app
fly open
```

### Environment Variables
Set these in Fly.io dashboard or via CLI:
- `DATABASE_URL`: Neon PostgreSQL connection string (postgresql://...)
- `JWT_SECRET`: JWT signing secret

Example:
```bash
fly secrets set DATABASE_URL="postgresql://user:pass@host.neon.tech/dbname?sslmode=require"
fly secrets set JWT_SECRET="your-jwt-secret"
```

### Scale Down (Free Tier)
```bash
# Scale to 0 machines when not in use (saves credits)
fly scale count 0

# Scale back up
fly scale count 1
```

### Troubleshooting
```bash
# Check deployed machines
fly machine list

# Restart a machine
fly machine restart <machine-id>

# Check connection to database
fly ssh console
psql "$DATABASE_URL" -c "SELECT 1"

# View recent logs
fly logs

# Redeploy after config changes
fly deploy
```

### URLs After Deployment
- **App**: https://graphql-bookstore.fly.dev
- **GraphQL Playground**: https://graphql-bookstore.fly.dev/
- **GraphQL Endpoint**: https://graphql-bookstore.fly.dev/graphql
- **Health Check**: https://graphql-bookstore.fly.dev/health
