import { CloseoutCapture } from "@/components/jobs/closeout-capture";

type PageProps = { params: Promise<{ id: string }> };

export default async function CloseoutPage({ params }: PageProps) {
  const { id } = await params;
  return <CloseoutCapture id={id} />;
}
