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
echo "$RESPONSE" | grep -q 'passwordHash' && echo "   ⚠️  BOLA vulnerability exposed (passwords leaked!)" || echo "   ✓ BOLA protected"
echo ""

echo "7. Testing Mass Assignment (Role Escalation)..."
TOKEN=$(curl -s -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d '{"query": "mutation { login(username: \"user\", password: \"password123\") { success token } }"}' \
  | grep -oP '"token":"[^"]+\"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
  RESPONSE=$(curl -s -X POST "$API_URL" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"query": "mutation { updateProfile(username: \"user\", role: \"admin\") { id role } }"}')
  echo "$RESPONSE" | grep -q '"role":"admin"' && echo "   ⚠️  Mass Assignment vulnerability (role changed to admin!)" || echo "   ✓ Mass Assignment protected"
fi
echo ""

echo "=========================================="
echo "  Test Complete                          "
echo "=========================================="
