import { CloseoutReady } from "@/components/jobs/closeout-ready";

type PageProps = { params: Promise<{ id: string }> };

export default async function ReadyPage({ params }: PageProps) {
  const { id } = await params;
  return <CloseoutReady id={id} />;
}
