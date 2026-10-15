import { GapResolution } from "@/components/jobs/gap-resolution";

type PageProps = { params: Promise<{ id: string }> };

export default async function GapsPage({ params }: PageProps) {
  const { id } = await params;
  return <GapResolution id={id} />;
}
