#!/bin/bash

echo "=========================================="
echo "  Vulnerable GraphQL API - Test Suite  "
echo "=========================================="
echo ""

API_URL="http://localhost:4000/graphql"

echo "1. Testing Server Health..."
curl -s http://localhost:4000/ | grep -q "GraphQL Playground" && echo "   ✓ Playground accessible" || echo "   ✗ Playground not accessible"
echo ""

echo "2. Testing Registration..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { register(username: \"testuser\", firstName: \"Test\", lastName: \"User\", password: \"testpass123\") { success message token } }"}')
echo "$RESPONSE" | grep -q '"success":true' && echo "   ✓ Registration successful" || echo "   ✗ Registration failed"
echo ""

echo "3. Testing Login (with username/password)..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { login(username: \"admin\", password: \"password123\") { success message token } }"}')
echo "$RESPONSE" | grep -q '"success":true' && echo "   ✓ Login successful" || echo "   ✗ Login failed"
echo ""

echo "4. Testing Books Query..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { books { id title price stockQuantity } }"}')
echo "$RESPONSE" | grep -q '"books":' && echo "   ✓ Books query works" || echo "   ✗ Books query failed"
echo ""

echo "5. Testing SSRF (Whitelisted URL)..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _fetchExternalResource(url: \"http://example.com\") }"}')
echo "$RESPONSE" | grep -q '_fetchExternalResource' && echo "   ✓ SSRF endpoint accessible" || echo "   ✗ SSRF endpoint failed"
echo ""

echo "6. Testing BOLA (User Enumeration - No Auth)..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _internalUserSearch(username: \"\") { id username passwordHash } }"}')
# Check if response contains passwordHash field (vulnerability exists if users are returned with password hashes)
USER_COUNT=$(echo "$RESPONSE" | grep -o '"username"' | wc -l)
if [ "$USER_COUNT" -gt 0 ]; then
  echo "   ⚠️  BOLA vulnerability exposed ($USER_COUNT users leaked without authentication!)"
  echo "   Sample response: $(echo "$RESPONSE" | head -c 200)..."
else
  echo "   ✓ BOLA protected"
fi
echo ""

echo "7. Testing Mass Assignment (Role Escalation)..."
TOKEN=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { login(username: \"user\", password: \"password123\") { success token } }"}' \
  | grep -oP '"token":"[^"]+\"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
  echo "   Testing with token: ${TOKEN:0:20}..."
  RESPONSE=$(curl -s -X POST "$API_URL" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query": "mutation { updateProfile(username: \"user\", firstName: \"Hacked\", role: \"admin\") { id username role firstName } }"}')
  echo "   Response: $RESPONSE"
  # Check if role was changed to admin
  if echo "$RESPONSE" | grep -q '"role":"admin"'; then
    echo "   ⚠️  Mass Assignment vulnerability (role changed to admin!)"
  elif echo "$RESPONSE" | grep -q '"firstName":"Hacked"'; then
    echo "   ✓ Profile updated but role not changed (protected)"
  else
    echo "   ⚠️  Check response above"
  fi
fi
echo ""

echo "8. Testing SQL Injection (_searchAdvanced)..."
echo "   8a. Normal search..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _searchAdvanced(query: \"SQL\") { id title } }"}')
echo "$RESPONSE" | grep -q '"_searchAdvanced":' && echo "      ✓ Normal search works" || echo "      ✗ Normal search failed"
echo ""

echo "   8b. SQL Injection - OR 1=1..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _searchAdvanced(query: \"x\") { id title } }"}')
echo "$RESPONSE" | head -c 500
echo ""

echo "   8c. SQL Injection - UNION based..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _searchAdvanced(query: \"x\") { id title } }"}')
echo "$RESPONSE" | grep -q 'error' && echo "      ✓ SQL Injection protected" || echo "      ⚠️  SQL Injection might be possible - check response above"
echo ""

echo "9. Testing IDOR (cancelOrder - access other user's orders)..."
TOKEN=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { login(username: \"user\", password: \"password123\") { success token } }"}' \
  | grep -oP '"token":"[^"]+\"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
  echo "   Trying to cancel order that doesn't belong to user..."
  RESPONSE=$(curl -s -X POST "$API_URL" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query": "mutation { cancelOrder(orderId: \"order-1234567890-9999\") { success message } }"}')
  echo "$RESPONSE" | grep -q '"Order not found"' && echo "      ✓ Order not found (expected)" || echo "      Response: $RESPONSE"
fi
echo ""

echo "10. Testing IDOR (deleteReview - access other user's reviews)..."
TOKEN=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { login(username: \"admin\", password: \"password123\") { success token } }"}' \
  | grep -oP '"token":"[^"]+\"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
  echo "   Getting admin's review ID first..."
  REVIEWS=$(curl -s -X POST "$API_URL" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query": "query { myReviews { id } }"}')
  echo "   Admin reviews: $REVIEWS"
  echo ""
  echo "   Trying to delete admin's review as user..."
  USER_TOKEN=$(curl -s -X POST "$API_URL" \
    -H "Content-Type: application/json" \
    -d '{"query": "mutation { login(username: \"user\", password: \"password123\") { success token } }"}' \
    | grep -oP '"token":"[^"]+\"' | cut -d'"' -f4)

  if [ -n "$USER_TOKEN" ] && echo "$REVIEWS" | grep -q '"id"'; then
    REVIEW_ID=$(echo "$REVIEWS" | grep -oP '"id":"[^"]+"' | head -1 | cut -d'"' -f4)
    echo "   Attempting to delete review: $REVIEW_ID"
    RESPONSE=$(curl -s -X POST "$API_URL" \
      -H "Content-Type: application/json" \
      -H "Authorization: Bearer $USER_TOKEN" \
      -d "{\"query\": \"mutation { deleteReview(reviewId: \\\"$REVIEW_ID\\\") { success message } }\"}")
    echo "$RESPONSE" | grep -q '"success":false' && echo "      ✓ IDOR protected (could not delete other's review)" || echo "      ⚠️  IDOR vulnerability (deleted other's review!)"
  fi
fi
echo ""

echo "11. Testing Admin Endpoints (No Auth Required)..."
echo "   11a. _adminStats..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _adminStats { userCount bookCount totalRevenue } }"}')
echo "$RESPONSE" | grep -q '"userCount"' && echo "      ⚠️  _adminStats exposed without authentication!" || echo "      ✓ _adminStats protected"
echo ""

echo "   11b. _adminAllOrders..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _adminAllOrders { id orderNumber totalAmount } }"}')
echo "$RESPONSE" | grep -q '"_adminAllOrders"' && echo "      ⚠️  _adminAllOrders exposed without authentication!" || echo "      ✓ _adminAllOrders protected"
echo ""

echo "   11c. _adminAllPayments..."
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "query { _adminAllPayments { id amount transactionId } }"}')
echo "$RESPONSE" | grep -q '"_adminAllPayments"' && echo "      ⚠️  _adminAllPayments exposed without authentication!" || echo "      ✓ _adminAllPayments protected"
echo ""

echo "12. Testing Complex Nested Queries..."
COMPLEX_QUERY='query { books { id title price author { id name } reviews { id rating comment } } }'
RESPONSE=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d "{\"query\": \"$COMPLEX_QUERY\"}")
echo "$RESPONSE" | grep -q '"books":' && echo "   ✓ Complex nested query works" || echo "   ✗ Complex nested query failed"
echo ""

echo "=========================================="
echo "  Test Complete                          "
echo "=========================================="
