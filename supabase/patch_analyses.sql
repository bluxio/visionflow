-- Run in Supabase SQL Editor if save_analysis fails on missing columns.

alter table public.analyses
  add column if not exists feedback jsonb not null default '[]'::jsonb;

alter table public.analyses
  add column if not exists video_path text;

-- Refresh PostgREST schema cache (Supabase)
notify pgrst, 'reload schema';
