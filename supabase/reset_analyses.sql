-- Run once in Supabase SQL Editor if analyses table has wrong columns.
-- Safe when the table is empty (no production data yet).

drop table if exists public.analyses cascade;

create table public.analyses (
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

create index analyses_client_created_idx
  on public.analyses (client_id, created_at desc);

alter table public.analyses enable row level security;
