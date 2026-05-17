-- Run in Supabase SQL Editor (one time)
-- Enables direct browser uploads; backend downloads with service role.

insert into storage.buckets (id, name, public, file_size_limit)
values ('workout-videos', 'workout-videos', false, 209715200)
on conflict (id) do update set file_size_limit = 209715200;

-- Allow anonymous uploads into this bucket only (MVP — tighten for production)
create policy "workout_videos_anon_insert"
on storage.objects for insert
to anon
with check (bucket_id = 'workout-videos');

create policy "workout_videos_anon_select"
on storage.objects for select
to anon
using (bucket_id = 'workout-videos');
