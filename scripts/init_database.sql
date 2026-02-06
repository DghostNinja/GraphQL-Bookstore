-- Initialize bookstore database schema
-- Run as postgres superuser or with appropriate privileges

-- Create database
DROP DATABASE IF EXISTS bookstore_db;
CREATE DATABASE bookstore_db;

\c bookstore_db;

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Users table with username-based authentication
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    first_name VARCHAR(100) NOT NULL,
    last_name VARCHAR(100) NOT NULL,
    role VARCHAR(20) DEFAULT 'user' CHECK (role IN ('user', 'staff', 'admin')),
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP,
    phone VARCHAR(20),
    address TEXT,
    city VARCHAR(100),
    state VARCHAR(50),
    zip_code VARCHAR(20),
    country VARCHAR(100)
);

-- Categories table
CREATE TABLE categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    parent_id INTEGER REFERENCES categories(id),
    is_active BOOLEAN DEFAULT true
);

-- Authors table
CREATE TABLE authors (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    bio TEXT,
    birth_date DATE,
    website VARCHAR(255),
    is_active BOOLEAN DEFAULT true
);

-- Books table
CREATE TABLE books (
    id SERIAL PRIMARY KEY,
    isbn VARCHAR(20) UNIQUE NOT NULL,
    title VARCHAR(500) NOT NULL,
    description TEXT,
    author_id INTEGER REFERENCES authors(id),
    category_id INTEGER REFERENCES categories(id),
    publisher VARCHAR(255),
    publication_date DATE,
    price DECIMAL(10, 2) NOT NULL,
    sale_price DECIMAL(10, 2),
    stock_quantity INTEGER DEFAULT 0,
    low_stock_threshold INTEGER DEFAULT 5,
    isbn_13 VARCHAR(20),
    language VARCHAR(50) DEFAULT 'English',
    pages INTEGER,
    format VARCHAR(50) DEFAULT 'Paperback',
    dimensions VARCHAR(50),
    weight DECIMAL(10, 2),
    rating_average DECIMAL(3, 2) DEFAULT 0,
    review_count INTEGER DEFAULT 0,
    is_featured BOOLEAN DEFAULT false,
    is_bestseller BOOLEAN DEFAULT false,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Reviews table
CREATE TABLE reviews (
    id SERIAL PRIMARY KEY,
    user_id UUID REFERENCES users(id),
    book_id INTEGER REFERENCES books(id),
    rating INTEGER CHECK (rating >= 1 AND rating <= 5),
    comment TEXT,
    is_verified_purchase BOOLEAN DEFAULT false,
    is_approved BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT unique_user_book_review UNIQUE (user_id, book_id)
);

-- Shopping carts table
CREATE TABLE shopping_carts (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Cart items table
CREATE TABLE cart_items (
    id SERIAL PRIMARY KEY,
    cart_id UUID REFERENCES shopping_carts(id),
    book_id INTEGER REFERENCES books(id),
    quantity INTEGER DEFAULT 1 CHECK (quantity > 0),
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT unique_cart_book UNIQUE (cart_id, book_id)
);

-- Orders table
CREATE TABLE orders (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),
    order_number VARCHAR(50) UNIQUE NOT NULL,
    status VARCHAR(50) DEFAULT 'pending' CHECK (status IN ('pending', 'processing', 'shipped', 'delivered', 'cancelled', 'refunded')),
    subtotal DECIMAL(10, 2) NOT NULL,
    tax_amount DECIMAL(10, 2) DEFAULT 0,
    shipping_amount DECIMAL(10, 2) DEFAULT 0,
    discount_amount DECIMAL(10, 2) DEFAULT 0,
    total_amount DECIMAL(10, 2) NOT NULL,
    shipping_address TEXT NOT NULL,
    billing_address TEXT NOT NULL,
    payment_method VARCHAR(50),
    payment_status VARCHAR(50) DEFAULT 'pending' CHECK (payment_status IN ('pending', 'processing', 'completed', 'failed', 'refunded')),
    tracking_number VARCHAR(100),
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    shipped_at TIMESTAMP,
    delivered_at TIMESTAMP
);

-- Order items table
CREATE TABLE order_items (
    id SERIAL PRIMARY KEY,
    order_id UUID REFERENCES orders(id),
    book_id INTEGER REFERENCES books(id),
    book_title VARCHAR(500),
    book_isbn VARCHAR(20),
    quantity INTEGER NOT NULL,
    unit_price DECIMAL(10, 2) NOT NULL,
    total_price DECIMAL(10, 2) NOT NULL
);

-- Coupons table
CREATE TABLE coupons (
    id SERIAL PRIMARY KEY,
    code VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    discount_type VARCHAR(20) NOT NULL CHECK (discount_type IN ('percentage', 'fixed')),
    discount_value DECIMAL(10, 2) NOT NULL,
    min_order_amount DECIMAL(10, 2) DEFAULT 0,
    max_discount_amount DECIMAL(10, 2),
    usage_limit INTEGER,
    usage_count INTEGER DEFAULT 0,
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Coupon usage tracking
CREATE TABLE coupon_usage (
    id SERIAL PRIMARY KEY,
    coupon_id INTEGER REFERENCES coupons(id),
    user_id UUID REFERENCES users(id),
    order_id UUID REFERENCES orders(id),
    discount_amount DECIMAL(10, 2) NOT NULL,
    used_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT unique_user_coupon_order UNIQUE (user_id, order_id)
);

-- Payment transactions table
CREATE TABLE payment_transactions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    order_id UUID REFERENCES orders(id),
    user_id UUID REFERENCES users(id),
    amount DECIMAL(10, 2) NOT NULL,
    currency VARCHAR(3) DEFAULT 'USD',
    payment_method VARCHAR(50),
    status VARCHAR(50) DEFAULT 'pending',
    transaction_id VARCHAR(255),
    gateway_response TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    processed_at TIMESTAMP
);

-- Audit logs table
CREATE TABLE audit_logs (
    id SERIAL PRIMARY KEY,
    user_id UUID REFERENCES users(id),
    action VARCHAR(100) NOT NULL,
    entity_type VARCHAR(50),
    entity_id VARCHAR(255),
    ip_address INET,
    user_agent TEXT,
    old_values JSONB,
    new_values JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for performance
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_role ON users(role);
CREATE INDEX idx_books_category ON books(category_id);
CREATE INDEX idx_books_author ON books(author_id);
CREATE INDEX idx_books_isbn ON books(isbn);
CREATE INDEX idx_reviews_user ON reviews(user_id);
CREATE INDEX idx_reviews_book ON reviews(book_id);
CREATE INDEX idx_orders_user ON orders(user_id);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_created_at ON orders(created_at);
CREATE INDEX idx_cart_items_cart ON cart_items(cart_id);

-- Insert default users (plaintext passwords for educational server)
-- VULNERABILITY: Storing plaintext passwords
INSERT INTO users (username, password_hash, first_name, last_name, role) VALUES
('admin', 'password123', 'Admin', 'User', 'admin'),
('staff', 'password123', 'Staff', 'Member', 'staff'),
('user', 'password123', 'Regular', 'User', 'user');

-- Insert sample categories
INSERT INTO categories (name, description) VALUES
('Fiction', 'Fictional literature'),
('Non-Fiction', 'Non-fictional works'),
('Science Fiction', 'Science fiction and fantasy'),
('Mystery', 'Mystery and thriller novels'),
('Biography', 'Biographical works'),
('Technical', 'Technical and programming books');

-- Insert sample authors
INSERT INTO authors (name, bio) VALUES
('John Smith', 'Bestselling fiction author'),
('Jane Doe', 'Technical writer and developer'),
('Bob Johnson', 'Science fiction novelist'),
('Alice Williams', 'Mystery thriller writer');

-- Insert sample books
INSERT INTO books (isbn, title, description, author_id, category_id, price, stock_quantity, is_featured) VALUES
('9780132350884', 'Clean Code', 'A handbook of agile software craftsmanship', 2, 6, 42.99, 25, true),
('9780201633610', 'Design Patterns', 'Elements of Reusable Object-Oriented Software', 2, 6, 54.99, 15, true),
('9780321125217', 'Domain-Driven Design', 'Tackling Complexity in Heart of Software', 2, 6, 49.99, 20, false),
('9780735619678', 'Code Complete', 'A Practical Handbook of Software Construction', 2, 6, 39.99, 30, false),
('9780345391803', 'The Hitchhiker''s Guide to the Galaxy', 'A sci-fi comedy classic', 3, 3, 14.99, 50, true),
('9780345391802', 'The Restaurant at the End of the Universe', 'Second book in trilogy', 3, 3, 14.99, 45, false);

-- Insert sample coupons
INSERT INTO coupons (code, description, discount_type, discount_value, min_order_amount, usage_limit) VALUES
('WELCOME10', 'Welcome discount for new users', 'percentage', 10.0, 0, 100),
('FLAT20', 'Flat $20 off orders over $100', 'fixed', 20.0, 100.0, 50),
('SUMMER25', 'Summer sale - 25% off', 'percentage', 25.0, 50.0, 200);

-- Insert sample reviews
INSERT INTO reviews (user_id, book_id, rating, comment, is_verified_purchase, is_approved) 
SELECT u.id, b.id, 5, 'Excellent book! Highly recommended.', true, true
FROM users u, books b
WHERE u.username = 'user' AND b.id IN (1, 2);

-- Create shopping carts for users
INSERT INTO shopping_carts (user_id) 
SELECT id FROM users WHERE username = 'user';

-- Add items to cart
INSERT INTO cart_items (cart_id, book_id, quantity)
SELECT sc.id, b.id, 2
FROM shopping_carts sc, books b, users u
WHERE u.username = 'user' AND sc.user_id = u.id AND b.id = 1;

-- Create sample orders
INSERT INTO orders (user_id, order_number, status, subtotal, total_amount, shipping_address, billing_address, payment_status)
SELECT u.id, 'ORD-001', 'delivered', 42.99, 45.99, '123 Test St', '123 Test St', 'completed'
FROM users u WHERE u.username = 'user';

-- Add order items
INSERT INTO order_items (order_id, book_id, book_title, book_isbn, quantity, unit_price, total_price)
SELECT o.id, b.id, b.title, b.isbn, 1, b.price, b.price
FROM orders o, books b, users u
WHERE u.username = 'user' AND o.user_id = u.id AND b.id = 1;

-- Add audit logs
INSERT INTO audit_logs (user_id, action, entity_type, entity_id, ip_address) 
SELECT id, 'LOGIN', 'user', id::text, '127.0.0.1'::inet
FROM users;

-- Webhooks table for SSRF vulnerabilities
CREATE TABLE webhooks (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),
    url VARCHAR(500) NOT NULL,
    events JSONB,
    secret VARCHAR(255),
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_triggered TIMESTAMP,
    failure_count INTEGER DEFAULT 0
);

-- Insert sample webhook for testing SSRF
INSERT INTO webhooks (user_id, url, events, secret) 
SELECT id, 'http://example.com/webhook', '["ORDER_CREATED"]', 'webhook_secret_123'
FROM users WHERE username = 'admin';

-- Search logs for injection testing
CREATE TABLE search_logs (
    id SERIAL PRIMARY KEY,
    user_id UUID REFERENCES users(id),
    query TEXT NOT NULL,
    filters JSONB,
    results_count INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    execution_time_ms INTEGER
);

-- Admin-only: System configuration (exposed via BOLA)
CREATE TABLE system_config (
    id SERIAL PRIMARY KEY,
    key VARCHAR(100) UNIQUE NOT NULL,
    value TEXT NOT NULL,
    is_sensitive BOOLEAN DEFAULT false,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Insert sensitive config
INSERT INTO system_config (key, value, is_sensitive) VALUES
('jwt_secret', 'CHANGE_ME_IN_PRODUCTION_real_jwt_secret_key_2024', true),
('db_password', 'bookstore_password', true),
('stripe_api_key', 'sk_test_123456789', true),
('aws_access_key', 'AKIAIOSFODNN7EXAMPLE', true);

-- Grant privileges
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO public;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO public;
