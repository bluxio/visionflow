-- Workout Form Coach — Supabase schema
-- Run in Supabase SQL Editor or via migration tool.

create extension if not exists "pgcrypto";

create table if not exists public.analyses (
  id uuid primary key default gen_random_uuid(),
  client_id text not null,
  exercise_type text not null check (
    exercise_type in ('squat', 'deadlift', 'bench_press', 'barbell_row')
  ),
  overall_score numeric(5, 2) not null,
  rep_count integer not null default 0,
  feedback jsonb not null default '[]'::jsonb,
  recommendations jsonb not null default '[]'::jsonb,
  severity_max text not null default 'info' check (
    severity_max in ('info', 'warning', 'critical')
  ),
  video_path text,
  created_at timestamptz not null default now()
);

create index if not exists analyses_client_created_idx
  on public.analyses (client_id, created_at desc);

-- Backend uses service role key; restrict direct client access.
alter table public.analyses enable row level security;

-- No public policies: only service role (backend) reads/writes.
