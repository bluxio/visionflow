-- Run in Supabase SQL Editor (one time)
-- Bucket id must match NEXT_PUBLIC_SUPABASE_STORAGE_BUCKET / SUPABASE_STORAGE_BUCKET (default: uploads)

insert into storage.buckets (id, name, public, file_size_limit)
values ('uploads', 'uploads', false, 209715200)
on conflict (id) do update set file_size_limit = 209715200;

-- Allow anonymous uploads into this bucket only (MVP — tighten for production)
create policy "uploads_anon_insert"
on storage.objects for insert
to anon
with check (bucket_id = 'uploads');

create policy "uploads_anon_select"
on storage.objects for select
to anon
using (bucket_id = 'uploads');
